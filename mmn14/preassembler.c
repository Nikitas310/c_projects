#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "assembler.h"
#include "preassembler.h"
#include "macro_table.h"
#include "opcode_table.h"
#include "utils.h"
#include "errors.h"

static int get_first_token(
    const char *line,
    char *token
)
{
    int i;
    int j;

    i = 0;
    j = 0;

    while (line[i] != '\0' && isspace((unsigned char)line[i])) {
        i++;
    }

    while (line[i] != '\0' && !isspace((unsigned char)line[i])) {
        token[j] = line[i];
        i++;
        j++;
    }

    token[j] = '\0';

    return i;
}

static int has_extra_text(
    const char *line,
    int index
)
{
    while (line[index] != '\0') {
        if (!isspace((unsigned char)line[index])) {
            return 1;
        }
        index++;
    }

    return 0;
}

static int get_second_token_after_index(
    const char *line,
    int index,
    char *token
)
{
    int j;

    j = 0;

    while (line[index] != '\0' && isspace((unsigned char)line[index])) {
        index++;
    }

    while (line[index] != '\0' && !isspace((unsigned char)line[index])) {
        token[j] = line[index];
        index++;
        j++;
    }

    token[j] = '\0';

    return index;
}

static int is_directive_name(const char *name)
{
    return strcmp(name, ".db") == 0 ||
        strcmp(name, ".dh") == 0 ||
        strcmp(name, ".dw") == 0 ||
        strcmp(name, ".asciz") == 0 ||
        strcmp(name, ".extern") == 0 ||
        strcmp(name, ".entry") == 0;
}

static int is_service_word(const char *name)
{
    return strcmp(name, "mcro") == 0 || strcmp(name, "mcroend") == 0;
}

static int is_reserved_macro_name(const char *name)
{
    return find_opcode(name) != NULL ||
        is_directive_name(name) ||
        is_service_word(name);
}

static int is_valid_macro_name(const char *name)
{
    int i;
    int length;

    if (name == NULL) {
        return 0;
    }

    length = (int)strlen(name);
    if (length == 0 || length > MAX_MACRO_NAME_LENGTH) {
        return 0;
    }

    if (!isalpha((unsigned char)name[0])) {
        return 0;
    }

    for (i = 1; i < length; i++) {
        if (!isalnum((unsigned char)name[i]) && name[i] != '_') {
            return 0;
        }
    }

    if (is_reserved_macro_name(name)) {
        return 0;
    }

    return 1;
}

static void write_macro_body(
    FILE *output_file,
    Macro *macro
)
{
    MacroLine *line;

    line = macro->lines;
    while (line != NULL) {
        fputs(line->text, output_file);
        line = line->next;
    }
}

static int report_preassembler_error(
    int line_number,
    ErrorCode code,
    const char *details
)
{
    report_error(line_number, code, details);
    return 0;
}

int run_preassembler(
    const char *base_name
)
{
    char input_filename[MAX_FILENAME_LENGTH];
    char output_filename[MAX_FILENAME_LENGTH];
    char line[MAX_LINE_LENGTH + 2];
    char first_token[MAX_LINE_LENGTH + 2];
    char second_token[MAX_LINE_LENGTH + 2];
    FILE *input_file;
    FILE *output_file;
    Macro *macros;
    Macro *current_macro;
    Macro *called_macro;
    int inside_macro;
    int line_number;
    int success;
    int first_token_end;
    int second_token_end;

    if (base_name == NULL ||
        strlen(base_name) + strlen(AS_EXTENSION) >= MAX_FILENAME_LENGTH ||
        strlen(base_name) + strlen(AM_EXTENSION) >= MAX_FILENAME_LENGTH) {
        printf("Error: file name is too long %s\n", base_name == NULL ? "" : base_name);
        return 0;
    }

    build_filename(base_name, AS_EXTENSION, input_filename);
    build_filename(base_name, AM_EXTENSION, output_filename);

    input_file = fopen(input_filename, "r");
    if (input_file == NULL) {
        printf("Error: cannot open file %s\n", input_filename);
        return 0;
    }

    output_file = fopen(output_filename, "w");
    if (output_file == NULL) {
        fclose(input_file);
        printf("Error: cannot open file %s\n", output_filename);
        return 0;
    }

    macros = NULL;
    current_macro = NULL;
    inside_macro = 0;
    line_number = 0;
    success = 1;

    while (fgets(line, sizeof(line), input_file) != NULL) {
        line_number++;
        first_token_end = get_first_token(line, first_token);

        if (inside_macro) {
            if (strcmp(first_token, "mcroend") == 0) {
                if (has_extra_text(line, first_token_end)) {
                    success = report_preassembler_error(
                        line_number,
                        ERROR_INTERNAL,
                        "extra text after mcroend"
                    );
                    break;
                }

                inside_macro = 0;
                current_macro = NULL;
                continue;
            }

            if (!add_line_to_macro(current_macro, line)) {
                success = report_preassembler_error(
                    line_number,
                    ERROR_MEMORY_ALLOCATION,
                    "macro line"
                );
                break;
            }

            continue;
        }

        if (strcmp(first_token, "mcro") == 0) {
            second_token_end = get_second_token_after_index(line, first_token_end, second_token);

            if (second_token[0] == '\0') {
                success = report_preassembler_error(
                    line_number,
                    ERROR_INVALID_LABEL,
                    "missing macro name"
                );
                break;
            }

            if (has_extra_text(line, second_token_end)) {
                success = report_preassembler_error(
                    line_number,
                    ERROR_INTERNAL,
                    "extra text after macro name"
                );
                break;
            }

            if (!is_valid_macro_name(second_token)) {
                success = report_preassembler_error(
                    line_number,
                    ERROR_INVALID_LABEL,
                    second_token
                );
                break;
            }

            if (find_macro(macros, second_token) != NULL) {
                success = report_preassembler_error(
                    line_number,
                    ERROR_DUPLICATE_LABEL,
                    second_token
                );
                break;
            }

            if (!add_macro(&macros, second_token)) {
                success = report_preassembler_error(
                    line_number,
                    ERROR_MEMORY_ALLOCATION,
                    second_token
                );
                break;
            }

            current_macro = find_macro(macros, second_token);
            inside_macro = 1;
            continue;
        }

        if (strcmp(first_token, "mcroend") == 0) {
            if (has_extra_text(line, first_token_end)) {
                success = report_preassembler_error(
                    line_number,
                    ERROR_INTERNAL,
                    "extra text after mcroend"
                );
                break;
            }
        }

        called_macro = find_macro(macros, first_token);
        if (called_macro != NULL) {
            if (has_extra_text(line, first_token_end)) {
                success = report_preassembler_error(
                    line_number,
                    ERROR_INTERNAL,
                    "extra text after macro call"
                );
                break;
            }

            write_macro_body(output_file, called_macro);
            continue;
        }

        fputs(line, output_file);
    }

    fclose(input_file);
    fclose(output_file);
    free_macro_table(macros);

    if (!success) {
        remove(output_filename);
        return 0;
    }

    return 1;
}

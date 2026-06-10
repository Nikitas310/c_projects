#include <stdio.h>
#include <string.h>
#include "assembler.h"
#include "second_pass.h"
#include "parser.h"
#include "utils.h"
#include "errors.h"

static void mark_second_pass_error(
    FirstPassContext *context,
    int line_number,
    ErrorCode code,
    const char *details
)
{
    report_error(line_number, code, details);
    context->has_errors = 1;
}

static void clear_unresolved(
    CodeWord *word
)
{
    word->unresolved_type = UNRESOLVED_NONE;
    word->unresolved_label[0] = '\0';
}

static void patch_j_address(
    CodeWord *word,
    int address
)
{
    word->word = (word->word & ~0x1ffffffu) |
        ((unsigned int)address & 0x1ffffffu);
}

static void patch_i_immediate(
    CodeWord *word,
    int immed
)
{
    word->word = (word->word & ~0xffffu) |
        ((unsigned int)immed & 0xffffu);
}

static int validate_entry_operand(
    FirstPassContext *context,
    ParsedLine *parsed,
    int line_number
)
{
    if (parsed->operand_count != 1) {
        mark_second_pass_error(context, line_number, ERROR_INTERNAL, "invalid entry operand count");
        return 0;
    }

    if (!is_valid_label(parsed->operands[0])) {
        mark_second_pass_error(context, line_number, ERROR_INVALID_LABEL, parsed->operands[0]);
        return 0;
    }

    return 1;
}

static void process_entry(
    FirstPassContext *context,
    ParsedLine *parsed,
    int line_number
)
{
    Symbol *symbol;

    if (!validate_entry_operand(context, parsed, line_number)) {
        return;
    }

    symbol = find_symbol(context->symbols, parsed->operands[0]);
    if (symbol == 0) {
        mark_second_pass_error(context, line_number, ERROR_INVALID_LABEL, parsed->operands[0]);
        return;
    }

    if ((symbol->attributes & SYMBOL_EXTERN) != 0) {
        mark_second_pass_error(context, line_number, ERROR_DUPLICATE_LABEL, parsed->operands[0]);
        return;
    }

    symbol->attributes |= SYMBOL_ENTRY;
}

static void resolve_j_label(
    FirstPassContext *context,
    CodeWord *word,
    int current_address,
    int line_number
)
{
    Symbol *symbol;

    symbol = find_symbol(context->symbols, word->unresolved_label);
    if (symbol == 0) {
        mark_second_pass_error(context, line_number, ERROR_INVALID_LABEL, word->unresolved_label);
        return;
    }

    if ((symbol->attributes & SYMBOL_EXTERN) != 0) {
        if (!add_external_usage(&context->external_usages, symbol->name, current_address)) {
            mark_second_pass_error(context, line_number, ERROR_MEMORY_ALLOCATION, symbol->name);
            return;
        }
        patch_j_address(word, 0);
        clear_unresolved(word);
        return;
    }

    patch_j_address(word, symbol->address);
    clear_unresolved(word);
}

static void resolve_branch_label(
    FirstPassContext *context,
    CodeWord *word,
    int current_address,
    int line_number
)
{
    Symbol *symbol;
    int offset;

    symbol = find_symbol(context->symbols, word->unresolved_label);
    if (symbol == 0) {
        mark_second_pass_error(context, line_number, ERROR_INVALID_LABEL, word->unresolved_label);
        return;
    }

    if ((symbol->attributes & SYMBOL_EXTERN) != 0) {
        mark_second_pass_error(context, line_number, ERROR_INVALID_LABEL, word->unresolved_label);
        return;
    }

    offset = symbol->address - current_address;
    if (offset < -32768 || offset > 32767) {
        mark_second_pass_error(context, line_number, ERROR_INVALID_NUMBER, word->unresolved_label);
        return;
    }

    patch_i_immediate(word, offset);
    clear_unresolved(word);
}

static void resolve_instruction(
    FirstPassContext *context,
    int current_address,
    int line_number
)
{
    CodeWord *word;

    word = find_code_word_by_address(&context->code_image, current_address);
    if (word == 0 || word->unresolved_type == UNRESOLVED_NONE) {
        return;
    }

    if (word->unresolved_type == UNRESOLVED_J_LABEL) {
        resolve_j_label(context, word, current_address, line_number);
    } else if (word->unresolved_type == UNRESOLVED_BRANCH_LABEL) {
        resolve_branch_label(context, word, current_address, line_number);
    }
}

int run_second_pass(
    const char *base_name,
    FirstPassContext *context
)
{
    char filename[MAX_FILENAME_LENGTH];
    char line[MAX_LINE_LENGTH + 2];
    FILE *file;
    ParsedLine parsed;
    int line_number;
    int current_address;

    if (context == 0 || context->has_errors != 0) {
        return 0;
    }

    if (base_name == 0 ||
        strlen(base_name) + strlen(AM_EXTENSION) >= MAX_FILENAME_LENGTH) {
        printf("Error: file name is too long %s\n", base_name == 0 ? "" : base_name);
        context->has_errors = 1;
        return 0;
    }

    build_filename(base_name, AM_EXTENSION, filename);
    file = fopen(filename, "r");
    if (file == 0) {
        printf("Error: cannot open file %s\n", filename);
        context->has_errors = 1;
        return 0;
    }

    line_number = 0;
    current_address = INITIAL_IC;

    while (fgets(line, sizeof(line), file) != 0) {
        line_number++;

        if (!parse_line(line, &parsed, line_number)) {
            context->has_errors = 1;
            continue;
        }

        if (parsed.type == LINE_EMPTY || parsed.type == LINE_COMMENT) {
            continue;
        }

        if (parsed.type == LINE_DIRECTIVE) {
            if (strcmp(parsed.name, ".entry") == 0) {
                process_entry(context, &parsed, line_number);
            }
            continue;
        }

        if (parsed.type == LINE_INSTRUCTION) {
            resolve_instruction(context, current_address, line_number);
            current_address += INSTRUCTION_SIZE;
        }
    }

    fclose(file);
    return !context->has_errors;
}

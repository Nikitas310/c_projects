#include <stdio.h>
#include <string.h>
#include "assembler.h"
#include "output_files.h"
#include "utils.h"

static void write_hex_byte(
    FILE *file,
    unsigned char value
)
{
    fprintf(file, "%02X", value);
}

static int has_entries(
    Symbol *symbols
)
{
    while (symbols != 0) {
        if ((symbols->attributes & SYMBOL_ENTRY) != 0) {
            return 1;
        }
        symbols = symbols->next;
    }

    return 0;
}

static void write_instruction_word(
    FILE *file,
    CodeWord *word
)
{
    int i;
    unsigned char byte_value;

    fprintf(file, "%04d", word->address);
    for (i = 0; i < INSTRUCTION_SIZE; i++) {
        byte_value = (unsigned char)((word->word >> (8 * i)) & 0xff);
        fprintf(file, " ");
        write_hex_byte(file, byte_value);
    }
    fprintf(file, "\n");
}

static void write_data_image(
    FILE *file,
    DataImage *image
)
{
    int i;
    int bytes_in_line;

    i = 0;
    while (i < image->count) {
        fprintf(file, "%04d", image->items[i].address);
        bytes_in_line = 0;

        while (i < image->count && bytes_in_line < 4) {
            fprintf(file, " ");
            write_hex_byte(file, image->items[i].value);
            i++;
            bytes_in_line++;
        }

        fprintf(file, "\n");
    }
}

static int write_object_file(
    const char *base_name,
    FirstPassContext *context
)
{
    char filename[MAX_FILENAME_LENGTH];
    FILE *file;
    int i;

    build_filename(base_name, OB_EXTENSION, filename);
    file = fopen(filename, "w");
    if (file == 0) {
        return 0;
    }

    fprintf(file, "%d %d\n", context->icf - INITIAL_IC, context->dcf);

    for (i = 0; i < context->code_image.count; i++) {
        write_instruction_word(file, &context->code_image.items[i]);
    }

    write_data_image(file, &context->data_image);

    if (ferror(file)) {
        fclose(file);
        return 0;
    }

    fclose(file);
    return 1;
}

static int write_entries_file(
    const char *base_name,
    FirstPassContext *context
)
{
    char filename[MAX_FILENAME_LENGTH];
    FILE *file;
    Symbol *symbol;

    if (!has_entries(context->symbols)) {
        return 1;
    }

    build_filename(base_name, ENT_EXTENSION, filename);
    file = fopen(filename, "w");
    if (file == 0) {
        return 0;
    }

    symbol = context->symbols;
    while (symbol != 0) {
        if ((symbol->attributes & SYMBOL_ENTRY) != 0) {
            fprintf(file, "%s %04d\n", symbol->name, symbol->address);
        }
        symbol = symbol->next;
    }

    if (ferror(file)) {
        fclose(file);
        return 0;
    }

    fclose(file);
    return 1;
}

static int write_externals_file(
    const char *base_name,
    FirstPassContext *context
)
{
    char filename[MAX_FILENAME_LENGTH];
    FILE *file;
    ExternalUsage *usage;

    if (context->external_usages == 0) {
        return 1;
    }

    build_filename(base_name, EXT_EXTENSION, filename);
    file = fopen(filename, "w");
    if (file == 0) {
        return 0;
    }

    usage = context->external_usages;
    while (usage != 0) {
        fprintf(file, "%s %04d\n", usage->name, usage->address);
        usage = usage->next;
    }

    if (ferror(file)) {
        fclose(file);
        return 0;
    }

    fclose(file);
    return 1;
}

static void remove_output_files(
    const char *base_name
)
{
    char filename[MAX_FILENAME_LENGTH];

    build_filename(base_name, OB_EXTENSION, filename);
    remove(filename);
    build_filename(base_name, ENT_EXTENSION, filename);
    remove(filename);
    build_filename(base_name, EXT_EXTENSION, filename);
    remove(filename);
}

int generate_output_files(
    const char *base_name,
    FirstPassContext *context
)
{
    if (base_name == 0 || context == 0 || context->has_errors != 0) {
        if (base_name != 0) {
            remove_output_files(base_name);
        }
        return 0;
    }

    if (strlen(base_name) + strlen(OB_EXTENSION) >= MAX_FILENAME_LENGTH ||
        strlen(base_name) + strlen(ENT_EXTENSION) >= MAX_FILENAME_LENGTH ||
        strlen(base_name) + strlen(EXT_EXTENSION) >= MAX_FILENAME_LENGTH) {
        return 0;
    }

    remove_output_files(base_name);

    if (!write_object_file(base_name, context) ||
        !write_entries_file(base_name, context) ||
        !write_externals_file(base_name, context)) {
        remove_output_files(base_name);
        return 0;
    }

    return 1;
}

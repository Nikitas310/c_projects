#include <stdio.h>
#include <string.h>
#include "assembler_module.h"
#include "assembler.h"
#include "preassembler.h"
#include "first_pass.h"
#include "second_pass.h"
#include "output_files.h"
#include "utils.h"

static void remove_output_files_for_base(
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

int get_base_name_from_as_filename(
    const char *input_filename,
    char *base_name
)
{
    int length;
    int base_length;

    if (input_filename == 0 || base_name == 0) {
        return 0;
    }

    length = (int)strlen(input_filename);
    if (length <= (int)strlen(AS_EXTENSION)) {
        return 0;
    }

    if (strcmp(input_filename + length - strlen(AS_EXTENSION), AS_EXTENSION) != 0) {
        return 0;
    }

    base_length = length - (int)strlen(AS_EXTENSION);
    if (base_length <= 0 || base_length >= MAX_FILENAME_LENGTH) {
        return 0;
    }

    strncpy(base_name, input_filename, base_length);
    base_name[base_length] = '\0';

    return 1;
}

int assemble_file(
    const char *base_name
)
{
    FirstPassContext context;
    int success;

    remove_output_files_for_base(base_name);

    if (!run_preassembler(base_name)) {
        remove_output_files_for_base(base_name);
        return 0;
    }

    if (!run_first_pass(base_name, &context)) {
        free_first_pass_context(&context);
        remove_output_files_for_base(base_name);
        return 0;
    }

    if (!run_second_pass(base_name, &context)) {
        free_first_pass_context(&context);
        remove_output_files_for_base(base_name);
        return 0;
    }

    success = generate_output_files(base_name, &context);
    free_first_pass_context(&context);

    return success;
}

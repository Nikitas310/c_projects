#include "assembler_module.h"
#include "preassembler.h"
#include "first_pass.h"
#include "second_pass.h"
#include "output_files.h"

int assemble_file(
    const char *base_name
)
{
    FirstPassContext context;
    int success;

    if (!run_preassembler(base_name)) {
        return 0;
    }

    if (!run_first_pass(base_name, &context)) {
        free_first_pass_context(&context);
        return 0;
    }

    if (!run_second_pass(base_name, &context)) {
        free_first_pass_context(&context);
        return 0;
    }

    success = generate_output_files(base_name, &context);
    free_first_pass_context(&context);

    return success;
}

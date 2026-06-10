#include <stdio.h>
#include <string.h>
#include "assembler.h"
#include "first_pass.h"
#include "second_pass.h"
#include "output_files.h"

static int tests_failed = 0;

static void check(int condition, const char *message)
{
    if (!condition) {
        printf("FAIL: %s\n", message);
        tests_failed++;
    }
}

static void make_filename(
    const char *base_name,
    const char *extension,
    char *result
)
{
    strcpy(result, base_name);
    strcat(result, extension);
}

static int write_file(
    const char *filename,
    const char *content
)
{
    FILE *file;

    file = fopen(filename, "w");
    if (file == 0) {
        return 0;
    }

    fputs(content, file);
    fclose(file);
    return 1;
}

static int file_exists(
    const char *filename
)
{
    FILE *file;

    file = fopen(filename, "r");
    if (file == 0) {
        return 0;
    }

    fclose(file);
    return 1;
}

static int read_file(
    const char *filename,
    char *buffer,
    int buffer_size
)
{
    FILE *file;
    int length;

    file = fopen(filename, "r");
    if (file == 0) {
        return 0;
    }

    length = (int)fread(buffer, 1, buffer_size - 1, file);
    buffer[length] = '\0';
    fclose(file);
    return 1;
}

static void cleanup_case(
    const char *base_name
)
{
    char filename[MAX_FILENAME_LENGTH];

    make_filename(base_name, AM_EXTENSION, filename);
    remove(filename);
    make_filename(base_name, OB_EXTENSION, filename);
    remove(filename);
    make_filename(base_name, ENT_EXTENSION, filename);
    remove(filename);
    make_filename(base_name, EXT_EXTENSION, filename);
    remove(filename);
}

static int prepare_and_generate(
    const char *base_name,
    const char *content,
    FirstPassContext *context
)
{
    char filename[MAX_FILENAME_LENGTH];

    cleanup_case(base_name);
    make_filename(base_name, AM_EXTENSION, filename);
    if (!write_file(filename, content)) {
        return 0;
    }

    if (!run_first_pass(base_name, context)) {
        return 0;
    }

    if (!run_second_pass(base_name, context)) {
        return 0;
    }

    return generate_output_files(base_name, context);
}

static int read_output(
    const char *base_name,
    const char *extension,
    char *buffer,
    int buffer_size
)
{
    char filename[MAX_FILENAME_LENGTH];

    make_filename(base_name, extension, filename);
    return read_file(filename, buffer, buffer_size);
}

static int output_exists(
    const char *base_name,
    const char *extension
)
{
    char filename[MAX_FILENAME_LENGTH];

    make_filename(base_name, extension, filename);
    return file_exists(filename);
}

static void test_ob_basic_code_only(void)
{
    FirstPassContext context;
    char output[512];

    check(prepare_and_generate(
        "out_basic",
        "MAIN: add $3,$5,$9\n"
        "hlt\n",
        &context
    ), "basic code generate succeeds");

    check(read_output("out_basic", OB_EXTENSION, output, sizeof(output)), "basic ob exists");
    check(!output_exists("out_basic", ENT_EXTENSION), "basic ent absent");
    check(!output_exists("out_basic", EXT_EXTENSION), "basic ext absent");
    check(strstr(output, "8 0\n") == output, "basic ob header");
    check(strstr(output, "0100 ") != 0, "basic address 0100");
    check(strstr(output, "0104 ") != 0, "basic address 0104");

    free_first_pass_context(&context);
    cleanup_case("out_basic");
}

static void test_ob_code_and_data(void)
{
    FirstPassContext context;
    char output[512];

    check(prepare_and_generate(
        "out_code_data",
        "MAIN: add $3,$5,$9\n"
        "hlt\n"
        "STR: .asciz \"a\"\n"
        "K: .dw 31\n",
        &context
    ), "code data generate succeeds");

    check(read_output("out_code_data", OB_EXTENSION, output, sizeof(output)),
        "code data ob exists");
    check(strstr(output, "8 6\n") == output, "code data header");
    check(strstr(output, "0108 ") != 0, "data address 0108");
    check(strstr(output, "0112 ") != 0, "data second row address 0112");

    free_first_pass_context(&context);
    cleanup_case("out_code_data");
}

static void test_ent_created(void)
{
    FirstPassContext context;
    char output[512];

    check(prepare_and_generate(
        "out_ent",
        ".entry K\n"
        "hlt\n"
        "K: .dw 31\n",
        &context
    ), "ent generate succeeds");

    check(read_output("out_ent", ENT_EXTENSION, output, sizeof(output)), "ent exists");
    check(strstr(output, "K 0104\n") != 0, "ent contains K");

    free_first_pass_context(&context);
    cleanup_case("out_ent");
}

static void test_ent_not_created_without_entries(void)
{
    FirstPassContext context;

    check(prepare_and_generate(
        "out_no_ent",
        "hlt\n"
        "K: .dw 31\n",
        &context
    ), "no ent generate succeeds");

    check(!output_exists("out_no_ent", ENT_EXTENSION), "ent not created");

    free_first_pass_context(&context);
    cleanup_case("out_no_ent");
}

static void test_ext_created(void)
{
    FirstPassContext context;
    char output[512];

    check(prepare_and_generate(
        "out_ext",
        ".extern val1\n"
        "la val1\n"
        "call val1\n",
        &context
    ), "ext generate succeeds");

    check(read_output("out_ext", EXT_EXTENSION, output, sizeof(output)), "ext exists");
    check(strstr(output, "val1 0100\n") != 0, "ext usage 100");
    check(strstr(output, "val1 0104\n") != 0, "ext usage 104");

    free_first_pass_context(&context);
    cleanup_case("out_ext");
}

static void test_ext_not_created_without_external_usages(void)
{
    FirstPassContext context;

    check(prepare_and_generate(
        "out_no_ext",
        ".extern val1\n"
        "hlt\n",
        &context
    ), "no ext generate succeeds");

    check(!output_exists("out_no_ext", EXT_EXTENSION), "ext not created");

    free_first_pass_context(&context);
    cleanup_case("out_no_ext");
}

static void test_no_outputs_on_errors(void)
{
    FirstPassContext context;

    cleanup_case("out_errors");
    check(!prepare_and_generate(
        "out_errors",
        ".entry MISSING\n"
        "hlt\n",
        &context
    ), "error flow fails");
    check(!generate_output_files("out_errors", &context), "generate rejects error context");
    check(!output_exists("out_errors", OB_EXTENSION), "error ob absent");
    check(!output_exists("out_errors", ENT_EXTENSION), "error ent absent");
    check(!output_exists("out_errors", EXT_EXTENSION), "error ext absent");

    free_first_pass_context(&context);
    cleanup_case("out_errors");
}

static void test_assignment_like_example(void)
{
    FirstPassContext context;
    char ob_output[2048];
    char ent_output[512];
    char ext_output[512];

    check(prepare_and_generate(
        "out_assignment",
        ".entry NEXT\n"
        ".extern wNumber\n"
        "STR: .asciz \"aBcd\"\n"
        "MAIN: add $3,$5,$9\n"
        "LOOP: ori $9,-5,$2\n"
        "la val1\n"
        "jmp NEXT\n"
        "NEXT: move $20,$4\n"
        "LIST: .db 6,-9\n"
        "bgt $4,$2,END\n"
        "la K\n"
        "sw $0,4,$10\n"
        "bne $31,$9,LOOP\n"
        "call val1\n"
        "jmp $4\n"
        "la wNumber\n"
        ".extern val1\n"
        ".dh 27056\n"
        "K: .dw 31,-12\n"
        "END: hlt\n"
        ".entry K\n",
        &context
    ), "assignment output generate succeeds");

    check(read_output("out_assignment", OB_EXTENSION, ob_output, sizeof(ob_output)),
        "assignment ob exists");
    check(read_output("out_assignment", ENT_EXTENSION, ent_output, sizeof(ent_output)),
        "assignment ent exists");
    check(read_output("out_assignment", EXT_EXTENSION, ext_output, sizeof(ext_output)),
        "assignment ext exists");

    check(strstr(ob_output, "52 17\n") == ob_output, "assignment ob header");
    check(strstr(ent_output, "NEXT 0116\n") != 0, "assignment ent NEXT");
    check(strstr(ent_output, "K 0161\n") != 0, "assignment ent K");
    check(strstr(ext_output, "val1 0108\n") != 0, "assignment ext val1 108");
    check(strstr(ext_output, "val1 0136\n") != 0, "assignment ext val1 136");
    check(strstr(ext_output, "wNumber 0144\n") != 0, "assignment ext wNumber");

    free_first_pass_context(&context);
    cleanup_case("out_assignment");
}

int main(void)
{
    test_ob_basic_code_only();
    test_ob_code_and_data();
    test_ent_created();
    test_ent_not_created_without_entries();
    test_ext_created();
    test_ext_not_created_without_external_usages();
    test_no_outputs_on_errors();
    test_assignment_like_example();

    if (tests_failed != 0) {
        printf("%d output file tests failed\n", tests_failed);
        return 1;
    }

    printf("All output file tests passed\n");
    return 0;
}

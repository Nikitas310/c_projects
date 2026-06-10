#include <stdio.h>
#include <string.h>
#include "assembler.h"
#include "preassembler.h"

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
    if (file == NULL) {
        return 0;
    }

    fputs(content, file);
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
    if (file == NULL) {
        return 0;
    }

    length = (int)fread(buffer, 1, buffer_size - 1, file);
    buffer[length] = '\0';

    fclose(file);
    return 1;
}

static int file_exists(
    const char *filename
)
{
    FILE *file;

    file = fopen(filename, "r");
    if (file == NULL) {
        return 0;
    }

    fclose(file);
    return 1;
}

static void cleanup_case(
    const char *base_name
)
{
    char filename[MAX_FILENAME_LENGTH];

    make_filename(base_name, AS_EXTENSION, filename);
    remove(filename);
    make_filename(base_name, AM_EXTENSION, filename);
    remove(filename);
}

static void test_valid_macro(void)
{
    char input_filename[MAX_FILENAME_LENGTH];
    char output_filename[MAX_FILENAME_LENGTH];
    char output[512];
    const char *base_name;
    const char *expected;

    base_name = "valid_macro";
    expected = "\nadd $1,$2,$3\nsub $4,$5,$6\n";

    cleanup_case(base_name);
    make_filename(base_name, AS_EXTENSION, input_filename);
    make_filename(base_name, AM_EXTENSION, output_filename);

    check(write_file(input_filename,
        "mcro HELLO\n"
        "add $1,$2,$3\n"
        "sub $4,$5,$6\n"
        "mcroend\n"
        "\n"
        "HELLO\n"), "write valid macro input");

    check(run_preassembler(base_name), "valid macro preassembler succeeds");
    check(read_file(output_filename, output, sizeof(output)), "valid macro output exists");
    check(strcmp(output, expected) == 0, "valid macro output content");

    cleanup_case(base_name);
}

static void test_assignment_example(void)
{
    char input_filename[MAX_FILENAME_LENGTH];
    char output_filename[MAX_FILENAME_LENGTH];
    char output[512];
    const char *base_name;
    const char *expected;

    base_name = "assignment_example";
    expected =
        "MAIN: add $3,$5,$9\n"
        "LOOP: ori $9,-5,$2\n"
        " la val1\n"
        " jmp NEXT\n"
        "NEXT: move $20,$4\n";

    cleanup_case(base_name);
    make_filename(base_name, AS_EXTENSION, input_filename);
    make_filename(base_name, AM_EXTENSION, output_filename);

    check(write_file(input_filename,
        "MAIN: add $3,$5,$9\n"
        "mcro GEN_MAC\n"
        " la val1\n"
        " jmp NEXT\n"
        "mcroend\n"
        "LOOP: ori $9,-5,$2\n"
        " GEN_MAC\n"
        "NEXT: move $20,$4\n"), "write assignment example input");

    check(run_preassembler(base_name), "assignment example preassembler succeeds");
    check(read_file(output_filename, output, sizeof(output)), "assignment example output exists");
    check(strcmp(output, expected) == 0, "assignment example output content");

    cleanup_case(base_name);
}

static void test_labeled_macro_text_remains(void)
{
    char input_filename[MAX_FILENAME_LENGTH];
    char output_filename[MAX_FILENAME_LENGTH];
    char output[512];
    const char *base_name;
    const char *expected;

    base_name = "labeled_macro_call";
    expected = "LABEL: TEST\n";

    cleanup_case(base_name);
    make_filename(base_name, AS_EXTENSION, input_filename);
    make_filename(base_name, AM_EXTENSION, output_filename);

    check(write_file(input_filename,
        "mcro TEST\n"
        "add $1,$2,$3\n"
        "mcroend\n"
        "LABEL: TEST\n"), "write labeled macro call input");

    check(run_preassembler(base_name), "labeled macro line is not preassembler error");
    check(read_file(output_filename, output, sizeof(output)), "labeled macro output exists");
    check(strcmp(output, expected) == 0, "labeled macro line remains unchanged");

    cleanup_case(base_name);
}

static void test_error_case(
    const char *base_name,
    const char *content,
    const char *message
)
{
    char input_filename[MAX_FILENAME_LENGTH];
    char output_filename[MAX_FILENAME_LENGTH];

    cleanup_case(base_name);
    make_filename(base_name, AS_EXTENSION, input_filename);
    make_filename(base_name, AM_EXTENSION, output_filename);

    check(write_file(input_filename, content), message);
    check(!run_preassembler(base_name), "preassembler rejects invalid macro file");
    check(!file_exists(output_filename), "invalid macro file leaves no am file");

    cleanup_case(base_name);
}

static void test_reserved_name(void)
{
    test_error_case(
        "reserved_opcode",
        "mcro add\n"
        "sub $4,$5,$6\n"
        "mcroend\n",
        "write reserved opcode input"
    );

    test_error_case(
        "reserved_directive",
        "mcro .db\n"
        "sub $4,$5,$6\n"
        "mcroend\n",
        "write reserved directive input"
    );
}

static void test_extra_text(void)
{
    test_error_case(
        "extra_text_macro",
        "mcro TEST abc\n"
        "add $1,$2,$3\n"
        "mcroend\n",
        "write extra text after macro name input"
    );

    test_error_case(
        "extra_text_macroend",
        "mcro TEST\n"
        "add $1,$2,$3\n"
        "mcroend abc\n",
        "write extra text after macroend input"
    );

    test_error_case(
        "extra_text_call",
        "mcro TEST\n"
        "add $1,$2,$3\n"
        "mcroend\n"
        "TEST abc\n",
        "write extra text after macro call input"
    );
}

int main(void)
{
    test_valid_macro();
    test_assignment_example();
    test_labeled_macro_text_remains();
    test_reserved_name();
    test_extra_text();

    if (tests_failed != 0) {
        printf("%d phase 2 tests failed\n", tests_failed);
        return 1;
    }

    printf("All phase 2 tests passed\n");
    return 0;
}

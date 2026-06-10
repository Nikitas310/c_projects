#include <stdio.h>
#include <string.h>
#include "assembler.h"
#include "assembler_module.h"
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

static int file_exists_name(
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

static int output_exists(
    const char *base_name,
    const char *extension
)
{
    char filename[MAX_FILENAME_LENGTH];

    make_filename(base_name, extension, filename);
    return file_exists_name(filename);
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
    make_filename(base_name, OB_EXTENSION, filename);
    remove(filename);
    make_filename(base_name, ENT_EXTENSION, filename);
    remove(filename);
    make_filename(base_name, EXT_EXTENSION, filename);
    remove(filename);
}

static int write_am(
    const char *base_name,
    const char *content
)
{
    char filename[MAX_FILENAME_LENGTH];

    cleanup_case(base_name);
    make_filename(base_name, AM_EXTENSION, filename);
    return write_file(filename, content);
}

static int write_as(
    const char *base_name,
    const char *content
)
{
    char filename[MAX_FILENAME_LENGTH];

    cleanup_case(base_name);
    make_filename(base_name, AS_EXTENSION, filename);
    return write_file(filename, content);
}

static int run_first_pass_case(
    const char *base_name,
    const char *content,
    FirstPassContext *context
)
{
    if (!write_am(base_name, content)) {
        return 0;
    }

    return run_first_pass(base_name, context);
}

static void test_cli_filename_handling(void)
{
    char base_name[MAX_FILENAME_LENGTH];

    check(get_base_name_from_as_filename("test.as", base_name), "test.as accepted");
    check(strcmp(base_name, "test") == 0, "test.as base");
    check(get_base_name_from_as_filename("dir/test.as", base_name), "dir/test.as accepted");
    check(strcmp(base_name, "dir/test") == 0, "dir/test.as base");
    check(!get_base_name_from_as_filename("test", base_name), "test rejected");
    check(!get_base_name_from_as_filename("test.am", base_name), "test.am rejected");
    check(!get_base_name_from_as_filename("test.txt", base_name), "test.txt rejected");
    check(!get_base_name_from_as_filename(".as", base_name), ".as rejected");
}

static void test_continue_after_bad_file(void)
{
    check(write_as("val_bad", ".entry MISSING\nhlt\n"), "write bad as");
    check(write_as("val_good", "hlt\n"), "write good as");

    check(!assemble_file("val_bad"), "bad assemble fails");
    check(assemble_file("val_good"), "good assemble succeeds after bad");
    check(!output_exists("val_bad", OB_EXTENSION), "bad ob absent");
    check(output_exists("val_good", OB_EXTENSION), "good ob exists");

    cleanup_case("val_bad");
    cleanup_case("val_good");
}

static void test_line_too_long(void)
{
    FirstPassContext context;
    char long_line[MAX_LINE_LENGTH + 16];
    char content[256];
    int i;

    for (i = 0; i < MAX_LINE_LENGTH + 5; i++) {
        long_line[i] = 'a';
    }
    long_line[MAX_LINE_LENGTH + 5] = '\0';
    strcpy(content, long_line);
    strcat(content, "\nGOOD: hlt\n");

    check(!run_first_pass_case("val_long", content, &context), "long line fails");
    check(context.has_errors, "long line has error");
    check(find_symbol(context.symbols, "GOOD") != 0, "line after long line still parsed");

    free_first_pass_context(&context);
    cleanup_case("val_long");
}

static void test_invalid_labels(void)
{
    FirstPassContext context;

    check(!run_first_pass_case(
        "val_labels",
        "1ABC: hlt\n"
        "_bad: hlt\n"
        "bad_label: hlt\n"
        "TOO_LONG_LABEL_NAME_MORE_THAN_31_CHARS: hlt\n"
        "add: .db 1\n"
        "sub: hlt\n"
        ".db: hlt\n"
        ".entry: hlt\n"
        ".extern: hlt\n"
        "mcro: hlt\n"
        "mcroend: hlt\n"
        "Add: hlt\n",
        &context
    ), "invalid labels fail");
    check(context.has_errors, "invalid labels has errors");
    check(find_symbol(context.symbols, "Add") != 0, "case sensitive label accepted");

    free_first_pass_context(&context);
    cleanup_case("val_labels");
}

static void test_duplicate_labels(void)
{
    FirstPassContext context;

    check(!run_first_pass_case("val_dups", "A: .db 1\nA: hlt\n", &context),
        "duplicate labels fail");
    check(context.has_errors, "duplicate labels has errors");
    free_first_pass_context(&context);
    cleanup_case("val_dups");

    check(!run_first_pass_case("val_ext_conflict1", ".extern EXT\nEXT: hlt\n", &context),
        "extern then local fails");
    free_first_pass_context(&context);
    cleanup_case("val_ext_conflict1");

    check(!run_first_pass_case("val_ext_conflict2", "EXT: hlt\n.extern EXT\n", &context),
        "local then extern fails");
    free_first_pass_context(&context);
    cleanup_case("val_ext_conflict2");
}

static void test_entry_extern_errors(void)
{
    FirstPassContext context;

    check(!run_first_pass_case(
        "val_entry_extern",
        ".entry\n"
        ".entry A,B\n"
        ".entry 1BAD\n"
        ".extern\n"
        ".extern A,B\n"
        ".extern 1BAD\n",
        &context
    ), "entry extern syntax errors fail first pass");
    free_first_pass_context(&context);
    cleanup_case("val_entry_extern");

    check(run_first_pass_case("val_entry_missing", ".entry MISSING\nhlt\n", &context),
        "missing entry first pass ok");
    check(!run_second_pass("val_entry_missing", &context), "missing entry second pass fails");
    free_first_pass_context(&context);
    cleanup_case("val_entry_missing");

    check(run_first_pass_case("val_repeat_extern", ".extern EXT\n.extern EXT\n", &context),
        "repeat extern ok");
    free_first_pass_context(&context);
    cleanup_case("val_repeat_extern");

    check(run_first_pass_case("val_ignored_extern_label", "ABC: .extern EXT\n", &context),
        "extern label ignored pass");
    check(find_symbol(context.symbols, "ABC") == 0, "extern leading label ignored");
    check(find_symbol(context.symbols, "EXT") != 0, "extern operand created");
    free_first_pass_context(&context);
    cleanup_case("val_ignored_extern_label");
}

static void test_data_range_errors(void)
{
    FirstPassContext context;

    check(!run_first_pass_case(
        "val_data_errors",
        ".db\n"
        ".db 128\n"
        ".db -129\n"
        ".db abc\n"
        ".db 1,,2\n"
        ".db ,1\n"
        ".db 1,\n"
        ".dh 32768\n"
        ".dh -32769\n"
        ".dh abc\n"
        ".dw 2147483648\n"
        ".dw -2147483649\n"
        ".dw abc\n",
        &context
    ), "data errors fail");
    check(context.has_errors, "data errors has errors");

    free_first_pass_context(&context);
    cleanup_case("val_data_errors");
}

static void test_asciz_errors(void)
{
    FirstPassContext context;

    check(run_first_pass_case(
        "val_asciz_ok",
        ".asciz \"hello\"\n.asciz \"\"\n.asciz \"hello world\"\n",
        &context
    ), "valid asciz pass");
    free_first_pass_context(&context);
    cleanup_case("val_asciz_ok");

    check(!run_first_pass_case(
        "val_asciz_bad",
        ".asciz\n"
        ".asciz hello\n"
        ".asciz \"missing end\n"
        ".asciz \"ok\" extra\n"
        ".asciz \"a\", \"b\"\n",
        &context
    ), "invalid asciz fails");
    free_first_pass_context(&context);
    cleanup_case("val_asciz_bad");
}

static void test_instruction_operand_errors(void)
{
    FirstPassContext context;

    check(!run_first_pass_case(
        "val_instr_errors",
        "add $1,$2\n"
        "add $1,$2,$3,$4\n"
        "add 1,$2,$3\n"
        "add $32,$2,$3\n"
        "add $1,$-1,$3\n"
        "move $1\n"
        "move $1,$2,$3\n"
        "move 1,$2\n"
        "move $1,$32\n"
        "addi $1,$2,$3\n"
        "addi $1,999999,$2\n"
        "addi $1,5\n"
        "lw $1,abc,$2\n"
        "sw 1,4,$2\n"
        "beq $1,$2\n"
        "beq $1,$2,1BAD\n"
        "jmp\n"
        "jmp $32\n"
        "jmp 1BAD\n"
        "la $1\n"
        "call $1\n"
        "hlt $1\n",
        &context
    ), "instruction operand errors fail");
    free_first_pass_context(&context);
    cleanup_case("val_instr_errors");

    check(run_first_pass_case(
        "val_second_instr_errors",
        "beq $1,$2,MISSING\n.extern EXT\nbeq $1,$2,EXT\nla MISSING\ncall MISSING\n",
        &context
    ), "second pass instruction errors first pass ok");
    check(!run_second_pass("val_second_instr_errors", &context),
        "second pass instruction errors fail");
    free_first_pass_context(&context);
    cleanup_case("val_second_instr_errors");
}

static void test_unknown_names(void)
{
    FirstPassContext context;

    check(!run_first_pass_case("val_unknown", "unknown $1,$2\n.unknown 1\n", &context),
        "unknown names fail");
    free_first_pass_context(&context);
    cleanup_case("val_unknown");
}

static void test_comma_errors(void)
{
    FirstPassContext context;

    check(!run_first_pass_case(
        "val_commas",
        ".db ,1\n"
        ".db 1,\n"
        ".db 1,,2\n"
        "add $1,, $2\n"
        "add ,$1,$2,$3\n"
        "add $1,$2,$3,\n"
        ".entry ,K\n"
        ".extern EXT,\n",
        &context
    ), "comma errors fail");
    free_first_pass_context(&context);
    cleanup_case("val_commas");
}

static void test_no_outputs_on_errors(void)
{
    check(write_as("val_no_outputs", ".entry MISSING\nhlt\n"), "write no output as");
    check(!assemble_file("val_no_outputs"), "bad assemble fails");
    check(!output_exists("val_no_outputs", OB_EXTENSION), "bad ob absent");
    check(!output_exists("val_no_outputs", ENT_EXTENSION), "bad ent absent");
    check(!output_exists("val_no_outputs", EXT_EXTENSION), "bad ext absent");
    cleanup_case("val_no_outputs");
}

static void test_no_unresolved_after_second_pass(void)
{
    FirstPassContext context;

    check(run_first_pass_case(
        "val_unresolved",
        "START: beq $1,$2,END\njmp END\nEND: hlt\n",
        &context
    ), "unresolved first pass ok");
    check(run_second_pass("val_unresolved", &context), "unresolved second pass ok");
    check(!has_unresolved_code_words(&context.code_image), "no unresolved after second pass");

    free_first_pass_context(&context);
    cleanup_case("val_unresolved");
}

static void test_output_cleanup_after_failed_rebuild(void)
{
    check(write_as(
        "val_cleanup",
        ".extern EXT\n.entry K\nla EXT\nK: .dw 1\n"
    ), "write cleanup valid as");
    check(assemble_file("val_cleanup"), "cleanup valid assemble succeeds");
    check(output_exists("val_cleanup", OB_EXTENSION), "cleanup ob exists");
    check(output_exists("val_cleanup", ENT_EXTENSION), "cleanup ent exists");
    check(output_exists("val_cleanup", EXT_EXTENSION), "cleanup ext exists");

    check(write_as("val_cleanup", ".entry MISSING\nhlt\n"), "write cleanup invalid as");
    check(!assemble_file("val_cleanup"), "cleanup invalid assemble fails");
    check(!output_exists("val_cleanup", OB_EXTENSION), "cleanup ob removed");
    check(!output_exists("val_cleanup", ENT_EXTENSION), "cleanup ent removed");
    check(!output_exists("val_cleanup", EXT_EXTENSION), "cleanup ext removed");

    cleanup_case("val_cleanup");
}

int main(void)
{
    test_cli_filename_handling();
    test_continue_after_bad_file();
    test_line_too_long();
    test_invalid_labels();
    test_duplicate_labels();
    test_entry_extern_errors();
    test_data_range_errors();
    test_asciz_errors();
    test_instruction_operand_errors();
    test_unknown_names();
    test_comma_errors();
    test_no_outputs_on_errors();
    test_no_unresolved_after_second_pass();
    test_output_cleanup_after_failed_rebuild();

    if (tests_failed != 0) {
        printf("%d validation tests failed\n", tests_failed);
        return 1;
    }

    printf("All validation tests passed\n");
    return 0;
}

#include <stdio.h>
#include <string.h>
#include "assembler.h"
#include "first_pass.h"
#include "second_pass.h"

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

static void cleanup_case(
    const char *base_name
)
{
    char filename[MAX_FILENAME_LENGTH];

    make_filename(base_name, AM_EXTENSION, filename);
    remove(filename);
}

static int prepare_case(
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

    return run_first_pass(base_name, context);
}

static int j_address(unsigned int word)
{
    return (int)(word & 0x1ffffffu);
}

static int i_immediate(unsigned int word)
{
    return (int)(word & 0xffffu);
}

static void test_entry_success(void)
{
    FirstPassContext context;
    Symbol *symbol;

    check(prepare_case("sp_entry", ".entry K\nK: .dw 31\n", &context),
        "entry first pass succeeds");
    check(run_second_pass("sp_entry", &context), "entry second pass succeeds");

    symbol = find_symbol(context.symbols, "K");
    check(symbol != 0, "K exists");
    check(symbol != 0 && (symbol->attributes & SYMBOL_DATA) != 0, "K data");
    check(symbol != 0 && (symbol->attributes & SYMBOL_ENTRY) != 0, "K entry");

    free_first_pass_context(&context);
    cleanup_case("sp_entry");
}

static void test_entry_duplicate_ok(void)
{
    FirstPassContext context;
    Symbol *symbol;

    check(prepare_case(
        "sp_entry_dup",
        ".entry K\n.entry K\nK: .dw 31\n",
        &context
    ), "entry duplicate first pass succeeds");
    check(run_second_pass("sp_entry_dup", &context), "entry duplicate second pass succeeds");

    symbol = find_symbol(context.symbols, "K");
    check(symbol != 0 && (symbol->attributes & SYMBOL_ENTRY) != 0,
        "duplicate entry marks entry");

    free_first_pass_context(&context);
    cleanup_case("sp_entry_dup");
}

static void test_entry_missing_error(void)
{
    FirstPassContext context;

    check(prepare_case("sp_entry_missing", ".entry MISSING\nhlt\n", &context),
        "entry missing first pass succeeds");
    check(!run_second_pass("sp_entry_missing", &context), "missing entry fails second pass");
    check(context.has_errors, "missing entry has errors");

    free_first_pass_context(&context);
    cleanup_case("sp_entry_missing");
}

static void test_j_local_labels(void)
{
    FirstPassContext context;
    Symbol *next_symbol;
    Symbol *k_symbol;
    Symbol *func_symbol;

    check(prepare_case(
        "sp_j_local",
        "jmp NEXT\n"
        "la K\n"
        "call FUNC\n"
        "NEXT: hlt\n"
        "K: .dw 31\n"
        "FUNC: hlt\n",
        &context
    ), "j local first pass succeeds");
    check(run_second_pass("sp_j_local", &context), "j local second pass succeeds");

    next_symbol = find_symbol(context.symbols, "NEXT");
    k_symbol = find_symbol(context.symbols, "K");
    func_symbol = find_symbol(context.symbols, "FUNC");

    check(j_address(context.code_image.items[0].word) == next_symbol->address, "jmp NEXT patched");
    check(j_address(context.code_image.items[1].word) == k_symbol->address, "la K patched");
    check(j_address(context.code_image.items[2].word) == func_symbol->address, "call FUNC patched");
    check(context.code_image.items[0].unresolved_type == UNRESOLVED_NONE, "jmp resolved");
    check(context.code_image.items[1].unresolved_type == UNRESOLVED_NONE, "la resolved");
    check(context.code_image.items[2].unresolved_type == UNRESOLVED_NONE, "call resolved");

    free_first_pass_context(&context);
    cleanup_case("sp_j_local");
}

static void test_j_external_usage(void)
{
    FirstPassContext context;

    check(prepare_case(
        "sp_j_external",
        ".extern val1\n"
        "la val1\n"
        "call val1\n"
        "jmp val1\n",
        &context
    ), "j external first pass succeeds");
    check(run_second_pass("sp_j_external", &context), "j external second pass succeeds");

    check(j_address(context.code_image.items[0].word) == 0, "la external address zero");
    check(j_address(context.code_image.items[1].word) == 0, "call external address zero");
    check(j_address(context.code_image.items[2].word) == 0, "jmp external address zero");
    check(count_external_usages(context.external_usages) == 3, "external usage count");
    check(find_external_usage(context.external_usages, "val1", 100) != 0, "external usage 100");
    check(find_external_usage(context.external_usages, "val1", 104) != 0, "external usage 104");
    check(find_external_usage(context.external_usages, "val1", 108) != 0, "external usage 108");

    free_first_pass_context(&context);
    cleanup_case("sp_j_external");
}

static void test_branch_forward_backward(void)
{
    FirstPassContext context;

    check(prepare_case(
        "sp_branch",
        "START: beq $1,$2,END\n"
        "bne $1,$2,START\n"
        "END: hlt\n",
        &context
    ), "branch first pass succeeds");
    check(run_second_pass("sp_branch", &context), "branch second pass succeeds");

    check(i_immediate(context.code_image.items[0].word) == 8, "beq forward offset");
    check(i_immediate(context.code_image.items[1].word) == 0xfffc, "bne backward offset");
    check(context.code_image.items[0].unresolved_type == UNRESOLVED_NONE, "beq resolved");
    check(context.code_image.items[1].unresolved_type == UNRESOLVED_NONE, "bne resolved");

    free_first_pass_context(&context);
    cleanup_case("sp_branch");
}

static void test_branch_external_error(void)
{
    FirstPassContext context;

    check(prepare_case("sp_branch_ext", ".extern EXT\nbne $1,$2,EXT\n", &context),
        "branch external first pass succeeds");
    check(!run_second_pass("sp_branch_ext", &context), "branch external second pass fails");
    check(context.has_errors, "branch external has errors");

    free_first_pass_context(&context);
    cleanup_case("sp_branch_ext");
}

static void test_undefined_j_error(void)
{
    FirstPassContext context;

    check(prepare_case("sp_undef_j", "jmp MISSING\n", &context),
        "undefined j first pass succeeds");
    check(!run_second_pass("sp_undef_j", &context), "undefined j second pass fails");
    check(context.has_errors, "undefined j has errors");

    free_first_pass_context(&context);
    cleanup_case("sp_undef_j");
}

static void test_undefined_branch_error(void)
{
    FirstPassContext context;

    check(prepare_case("sp_undef_branch", "bne $1,$2,MISSING\n", &context),
        "undefined branch first pass succeeds");
    check(!run_second_pass("sp_undef_branch", &context), "undefined branch second pass fails");
    check(context.has_errors, "undefined branch has errors");

    free_first_pass_context(&context);
    cleanup_case("sp_undef_branch");
}

static void test_entry_external_conflict(void)
{
    FirstPassContext context;
    Symbol *symbol;

    check(prepare_case("sp_entry_ext", ".extern EXT\n.entry EXT\n", &context),
        "entry external first pass succeeds");
    check(!run_second_pass("sp_entry_ext", &context), "entry external second pass fails");

    symbol = find_symbol(context.symbols, "EXT");
    check(symbol != 0 && (symbol->attributes & SYMBOL_EXTERN) != 0, "EXT remains external");
    check(symbol != 0 && (symbol->attributes & SYMBOL_ENTRY) == 0, "EXT not entry");

    free_first_pass_context(&context);
    cleanup_case("sp_entry_ext");
}

int main(void)
{
    test_entry_success();
    test_entry_duplicate_ok();
    test_entry_missing_error();
    test_j_local_labels();
    test_j_external_usage();
    test_branch_forward_backward();
    test_branch_external_error();
    test_undefined_j_error();
    test_undefined_branch_error();
    test_entry_external_conflict();

    if (tests_failed != 0) {
        printf("%d second pass tests failed\n", tests_failed);
        return 1;
    }

    printf("All second pass tests passed\n");
    return 0;
}

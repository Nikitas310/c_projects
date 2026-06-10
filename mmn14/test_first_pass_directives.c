#include <stdio.h>
#include <string.h>
#include "assembler.h"
#include "first_pass.h"

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

static int run_case(
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

    return run_first_pass_directives_only(base_name, context);
}

static unsigned char data_value(FirstPassContext *context, int index)
{
    return context->data_image.items[index].value;
}

static void test_db(void)
{
    FirstPassContext context;
    Symbol *symbol;

    check(run_case("fp_db", "LIST: .db 6, -9\n", &context), "db pass succeeds");

    symbol = find_symbol(context.symbols, "LIST");
    check(context.dc == 2, "db dc");
    check(symbol != 0, "db symbol exists");
    check(symbol != 0 && symbol->address == 0, "db symbol address");
    check(symbol != 0 && (symbol->attributes & SYMBOL_DATA) != 0, "db symbol data");
    check(data_value(&context, 0) == 6, "db first byte");
    check(data_value(&context, 1) == 247, "db second byte");

    free_first_pass_context(&context);
    cleanup_case("fp_db");
}

static void test_dh(void)
{
    FirstPassContext context;

    check(run_case("fp_dh", "HALFS: .dh 27056, -1\n", &context), "dh pass succeeds");

    check(context.dc == 4, "dh dc");
    check(data_value(&context, 0) == 176, "dh 27056 low byte");
    check(data_value(&context, 1) == 105, "dh 27056 high byte");
    check(data_value(&context, 2) == 255, "dh -1 low byte");
    check(data_value(&context, 3) == 255, "dh -1 high byte");

    free_first_pass_context(&context);
    cleanup_case("fp_dh");
}

static void test_dw(void)
{
    FirstPassContext context;
    Symbol *symbol;

    check(run_case("fp_dw", "K: .dw 31, -12\n", &context), "dw pass succeeds");

    symbol = find_symbol(context.symbols, "K");
    check(context.dc == 8, "dw dc");
    check(symbol != 0 && symbol->address == 0, "dw symbol address");
    check(symbol != 0 && (symbol->attributes & SYMBOL_DATA) != 0, "dw symbol data");
    check(data_value(&context, 0) == 31, "dw 31 byte 0");
    check(data_value(&context, 1) == 0, "dw 31 byte 1");
    check(data_value(&context, 2) == 0, "dw 31 byte 2");
    check(data_value(&context, 3) == 0, "dw 31 byte 3");
    check(data_value(&context, 4) == 244, "dw -12 byte 0");
    check(data_value(&context, 5) == 255, "dw -12 byte 1");
    check(data_value(&context, 6) == 255, "dw -12 byte 2");
    check(data_value(&context, 7) == 255, "dw -12 byte 3");

    free_first_pass_context(&context);
    cleanup_case("fp_dw");
}

static void test_asciz(void)
{
    FirstPassContext context;
    Symbol *symbol;

    check(run_case("fp_asciz", "STR: .asciz \"aBcd\"\n", &context), "asciz pass succeeds");

    symbol = find_symbol(context.symbols, "STR");
    check(context.dc == 5, "asciz dc");
    check(symbol != 0 && symbol->address == 0, "asciz symbol address");
    check(data_value(&context, 0) == 'a', "asciz byte a");
    check(data_value(&context, 1) == 'B', "asciz byte B");
    check(data_value(&context, 2) == 'c', "asciz byte c");
    check(data_value(&context, 3) == 'd', "asciz byte d");
    check(data_value(&context, 4) == 0, "asciz terminator");

    free_first_pass_context(&context);
    cleanup_case("fp_asciz");
}

static void test_multiple_directives(void)
{
    FirstPassContext context;
    Symbol *str_symbol;
    Symbol *list_symbol;
    Symbol *k_symbol;

    check(run_case(
        "fp_multiple",
        "STR: .asciz \"aBcd\"\n"
        "LIST: .db 6,-9\n"
        ".dh 27056\n"
        "K: .dw 31,-12\n",
        &context
    ), "multiple directives pass succeeds");

    str_symbol = find_symbol(context.symbols, "STR");
    list_symbol = find_symbol(context.symbols, "LIST");
    k_symbol = find_symbol(context.symbols, "K");

    check(str_symbol != 0 && str_symbol->address == 0, "multiple STR address");
    check(list_symbol != 0 && list_symbol->address == 5, "multiple LIST address");
    check(k_symbol != 0 && k_symbol->address == 9, "multiple K address");
    check(context.dc == 17, "multiple dc");

    free_first_pass_context(&context);
    cleanup_case("fp_multiple");
}

static void test_extern(void)
{
    FirstPassContext context;
    Symbol *symbol;

    check(run_case("fp_extern", ".extern val1\n.extern val1\n", &context),
        "extern pass succeeds");

    symbol = find_symbol(context.symbols, "val1");
    check(symbol != 0, "extern symbol exists");
    check(symbol != 0 && symbol->address == 0, "extern symbol address");
    check(symbol != 0 && (symbol->attributes & SYMBOL_EXTERN) != 0, "extern attribute");
    check(!context.has_errors, "extern duplicate has no error");

    free_first_pass_context(&context);
    cleanup_case("fp_extern");
}

static void test_entry_skipped(void)
{
    FirstPassContext context;

    check(run_case("fp_entry", ".entry K\n", &context), "entry pass succeeds");

    check(find_symbol(context.symbols, "K") == 0, "entry does not create symbol");
    check(context.dc == 0, "entry dc unchanged");
    check(!context.has_errors, "entry no error");

    free_first_pass_context(&context);
    cleanup_case("fp_entry");
}

static void test_entry_and_extern_label_ignored(void)
{
    FirstPassContext context;
    Symbol *ext_symbol;

    check(run_case(
        "fp_entry_extern_labels",
        "ABC: .entry K\n"
        "XYZ: .extern EXT\n",
        &context
    ), "entry extern labels pass succeeds");

    ext_symbol = find_symbol(context.symbols, "EXT");
    check(find_symbol(context.symbols, "ABC") == 0, "entry label ignored");
    check(find_symbol(context.symbols, "XYZ") == 0, "extern label ignored");
    check(ext_symbol != 0 && (ext_symbol->attributes & SYMBOL_EXTERN) != 0,
        "extern operand created");

    free_first_pass_context(&context);
    cleanup_case("fp_entry_extern_labels");
}

static void test_errors(void)
{
    FirstPassContext context;

    check(!run_case(
        "fp_errors",
        ".db 999\n"
        ".dh 999999\n"
        ".db abc\n"
        ".extern 1BAD\n"
        ".entry 1BAD\n",
        &context
    ), "error case returns failure");

    check(context.has_errors, "error case has errors");

    free_first_pass_context(&context);
    cleanup_case("fp_errors");
}

int main(void)
{
    test_db();
    test_dh();
    test_dw();
    test_asciz();
    test_multiple_directives();
    test_extern();
    test_entry_skipped();
    test_entry_and_extern_label_ignored();
    test_errors();

    if (tests_failed != 0) {
        printf("%d first pass directive tests failed\n", tests_failed);
        return 1;
    }

    printf("All first pass directive tests passed\n");
    return 0;
}

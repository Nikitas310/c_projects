#include <stdio.h>
#include <string.h>
#include "assembler.h"
#include "symbol_table.h"
#include "macro_table.h"
#include "opcode_table.h"
#include "utils.h"
#include "errors.h"

static int tests_failed = 0;

static void check(int condition, const char *message)
{
    if (!condition) {
        printf("FAIL: %s\n", message);
        tests_failed++;
    }
}

static void test_symbol_table(void)
{
    Symbol *symbols;
    Symbol *data_symbol;
    Symbol *code_symbol;
    Symbol *extern_symbol;

    symbols = NULL;

    check(add_symbol(&symbols, "MAIN", 100, SYMBOL_CODE), "add code symbol");
    check(add_symbol(&symbols, "DATA1", 0, SYMBOL_DATA), "add data symbol");
    check(add_symbol(&symbols, "EXT1", 0, SYMBOL_EXTERN), "add extern symbol");
    check(!add_symbol(&symbols, "MAIN", 200, SYMBOL_DATA), "reject duplicate symbol");
    check(symbol_exists(symbols, "MAIN"), "symbol_exists finds existing symbol");
    check(!symbol_exists(symbols, "MISSING"), "symbol_exists rejects missing symbol");
    check(find_symbol(symbols, "DATA1") != NULL, "find_symbol finds data symbol");

    update_data_symbols(symbols, 100);

    data_symbol = find_symbol(symbols, "DATA1");
    code_symbol = find_symbol(symbols, "MAIN");
    extern_symbol = find_symbol(symbols, "EXT1");

    check(data_symbol != NULL && data_symbol->address == 100, "update data symbol address");
    check(code_symbol != NULL && code_symbol->address == 100, "do not update code symbol address");
    check(extern_symbol != NULL && extern_symbol->address == 0, "do not update extern symbol address");

    free_symbol_table(symbols);
}

static void test_macro_table(void)
{
    Macro *macros;
    Macro *macro;

    macros = NULL;

    check(add_macro(&macros, "M1"), "add macro");
    check(!add_macro(&macros, "M1"), "reject duplicate macro");

    macro = find_macro(macros, "M1");
    check(macro != NULL, "find macro");
    check(add_line_to_macro(macro, "add $1,$2,$3"), "add first macro line");
    check(add_line_to_macro(macro, "sub $4,$5,$6"), "add second macro line");
    check(macro != NULL && macro->lines != NULL, "macro has first line");
    check(macro != NULL && macro->lines != NULL &&
        strcmp(macro->lines->text, "add $1,$2,$3") == 0, "first macro line text");
    check(macro != NULL && macro->lines != NULL &&
        macro->lines->next != NULL &&
        strcmp(macro->lines->next->text, "sub $4,$5,$6") == 0, "second macro line text");
    check(find_macro(macros, "MISSING") == NULL, "missing macro not found");

    free_macro_table(macros);
}

static void test_opcode_table(void)
{
    OpcodeInfo *opcode;

    opcode = find_opcode("add");
    check(opcode != NULL && opcode->type == 'R' && opcode->opcode == 0 &&
        opcode->funct == 1, "find R opcode");

    opcode = find_opcode("bne");
    check(opcode != NULL && opcode->type == 'I' && opcode->opcode == 15,
        "find I opcode");

    opcode = find_opcode("hlt");
    check(opcode != NULL && opcode->type == 'J' && opcode->opcode == 63,
        "find J opcode");

    check(find_opcode("missing") == NULL, "missing opcode returns NULL");
}

static void test_utils(void)
{
    char str1[32];
    char str2[32];
    char filename[64];
    long value;

    strcpy(str1, "  hello world  ");
    trim(str1);
    check(strcmp(str1, "hello world") == 0, "trim removes surrounding spaces");

    strcpy(str2, "abc   ");
    remove_trailing_spaces(str2);
    check(strcmp(str2, "abc") == 0, "remove trailing spaces");
    check(strcmp(skip_spaces("   xyz"), "xyz") == 0, "skip spaces");

    check(is_valid_label("Label1"), "valid label");
    check(is_valid_label("A123456789012345678901234567890"), "31 char label");
    check(!is_valid_label("1Label"), "label cannot start with digit");
    check(!is_valid_label("bad_label"), "label cannot contain underscore");
    check(!is_valid_label("add"), "label cannot be opcode");
    check(!is_valid_label("db"), "label cannot be directive");
    check(!is_valid_label("mcro"), "label cannot be macro keyword");
    check(!is_valid_label("A1234567890123456789012345678901"), "label too long");

    check(is_valid_register("$0"), "register $0");
    check(is_valid_register("$31"), "register $31");
    check(!is_valid_register("$32"), "register $32 invalid");
    check(!is_valid_register("5"), "register without dollar invalid");
    check(!is_valid_register("$1a"), "register with letter invalid");

    check(is_valid_integer("123"), "positive integer");
    check(is_valid_integer("-7"), "negative integer");
    check(is_valid_integer("+12"), "signed positive integer");
    check(is_valid_integer("0"), "zero integer");
    check(!is_valid_integer("+"), "sign only invalid");
    check(!is_valid_integer("12a"), "integer with letter invalid");

    check(parse_integer("123", &value) && value == 123, "parse positive integer");
    check(parse_integer("-7", &value) && value == -7, "parse negative integer");
    check(parse_integer("+12", &value) && value == 12, "parse signed positive integer");
    check(parse_integer("0", &value) && value == 0, "parse zero integer");
    check(!parse_integer("12abc", &value), "parse rejects trailing text");
    check(!parse_integer("+", &value), "parse rejects sign only");
    check(!parse_integer("", &value), "parse rejects empty string");

    build_filename("prog", AM_EXTENSION, filename);
    check(strcmp(filename, "prog.am") == 0, "build filename with arbitrary extension");
}

static void test_constants(void)
{
    Symbol symbol;
    Macro macro;

    check(sizeof(symbol.name) == MAX_LABEL_LENGTH + 1, "symbol uses MAX_LABEL_LENGTH");
    check(sizeof(macro.name) == MAX_MACRO_NAME_LENGTH + 1,
        "macro uses MAX_MACRO_NAME_LENGTH");
    check(MAX_FILENAME_LENGTH == 256, "filename length constant");
    check(MAX_LINE_LENGTH == 80, "line length constant");
    check(strcmp(AS_EXTENSION, ".as") == 0, "as extension constant");
    check(strcmp(AM_EXTENSION, ".am") == 0, "am extension constant");
    check(strcmp(OB_EXTENSION, ".ob") == 0, "ob extension constant");
    check(strcmp(ENT_EXTENSION, ".ent") == 0, "ent extension constant");
    check(strcmp(EXT_EXTENSION, ".ext") == 0, "ext extension constant");
}

static void test_errors(void)
{
    report_error(15, ERROR_INVALID_LABEL, "ABC$");
}

int main(void)
{
    test_symbol_table();
    test_macro_table();
    test_opcode_table();
    test_utils();
    test_constants();
    test_errors();

    if (tests_failed != 0) {
        printf("%d tests failed\n", tests_failed);
        return 1;
    }

    printf("All phase 1 tests passed\n");
    return 0;
}

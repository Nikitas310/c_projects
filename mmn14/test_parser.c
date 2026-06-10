#include <stdio.h>
#include <string.h>
#include "parser.h"

static int tests_failed = 0;

static void check(int condition, const char *message)
{
    if (!condition) {
        printf("FAIL: %s\n", message);
        tests_failed++;
    }
}

static void test_valid_empty_comment(void)
{
    ParsedLine parsed;

    check(parse_line("   \t\n", &parsed, 1), "parse empty line");
    check(parsed.type == LINE_EMPTY, "empty line type");

    check(parse_line("  ; comment\n", &parsed, 2), "parse comment line");
    check(parsed.type == LINE_COMMENT, "comment line type");
}

static void test_valid_instruction(void)
{
    ParsedLine parsed;

    check(parse_line("MAIN: add $3, $5, $9\n", &parsed, 3), "parse instruction");
    check(parsed.type == LINE_INSTRUCTION, "instruction type");
    check(parsed.has_label, "instruction has label");
    check(strcmp(parsed.label, "MAIN") == 0, "instruction label");
    check(strcmp(parsed.name, "add") == 0, "instruction name");
    check(parsed.operand_count == 3, "instruction operand count");
    check(strcmp(parsed.operands[0], "$3") == 0, "instruction operand 1");
    check(strcmp(parsed.operands[1], "$5") == 0, "instruction operand 2");
    check(strcmp(parsed.operands[2], "$9") == 0, "instruction operand 3");

    check(parse_line("add $3,$5,$9\n", &parsed, 3), "parse compact instruction");
    check(parsed.type == LINE_INSTRUCTION, "compact instruction type");
    check(parsed.operand_count == 3, "compact instruction operand count");
    check(strcmp(parsed.operands[0], "$3") == 0, "compact instruction operand 1");
    check(strcmp(parsed.operands[1], "$5") == 0, "compact instruction operand 2");
    check(strcmp(parsed.operands[2], "$9") == 0, "compact instruction operand 3");
}

static void test_valid_directive_numbers(void)
{
    ParsedLine parsed;

    check(parse_line("LIST: .db 7, -57, 17, +9\n", &parsed, 4), "parse db directive");
    check(parsed.type == LINE_DIRECTIVE, "db directive type");
    check(parsed.has_label, "db directive has label");
    check(strcmp(parsed.label, "LIST") == 0, "db directive label");
    check(strcmp(parsed.name, ".db") == 0, "db directive name");
    check(parsed.operand_count == 4, "db directive operand count");
    check(strcmp(parsed.operands[0], "7") == 0, "db operand 1");
    check(strcmp(parsed.operands[1], "-57") == 0, "db operand 2");
    check(strcmp(parsed.operands[2], "17") == 0, "db operand 3");
    check(strcmp(parsed.operands[3], "+9") == 0, "db operand 4");

    check(parse_line(".db 7,-57,17,+9\n", &parsed, 4), "parse compact db directive");
    check(parsed.type == LINE_DIRECTIVE, "compact db directive type");
    check(parsed.operand_count == 4, "compact db operand count");
    check(strcmp(parsed.operands[0], "7") == 0, "compact db operand 1");
    check(strcmp(parsed.operands[1], "-57") == 0, "compact db operand 2");
    check(strcmp(parsed.operands[2], "17") == 0, "compact db operand 3");
    check(strcmp(parsed.operands[3], "+9") == 0, "compact db operand 4");
}

static void test_valid_asciz(void)
{
    ParsedLine parsed;

    check(parse_line("STR: .asciz \"hello world\"\n", &parsed, 5), "parse asciz directive");
    check(parsed.type == LINE_DIRECTIVE, "asciz directive type");
    check(parsed.has_label, "asciz has label");
    check(strcmp(parsed.label, "STR") == 0, "asciz label");
    check(strcmp(parsed.name, ".asciz") == 0, "asciz name");
    check(parsed.operand_count == 1, "asciz operand count");
    check(strcmp(parsed.operands[0], "\"hello world\"") == 0, "asciz operand");
}

static void test_valid_entry_extern(void)
{
    ParsedLine parsed;

    check(parse_line(".entry K\n", &parsed, 6), "parse entry");
    check(parsed.type == LINE_DIRECTIVE, "entry type");
    check(strcmp(parsed.name, ".entry") == 0, "entry name");
    check(parsed.operand_count == 1, "entry operand count");
    check(strcmp(parsed.operands[0], "K") == 0, "entry operand");

    check(parse_line(".extern val1\n", &parsed, 7), "parse extern");
    check(parsed.type == LINE_DIRECTIVE, "extern type");
    check(strcmp(parsed.name, ".extern") == 0, "extern name");
    check(parsed.operand_count == 1, "extern operand count");
    check(strcmp(parsed.operands[0], "val1") == 0, "extern operand");
}

static void test_invalid_label(void)
{
    ParsedLine parsed;

    check(!parse_line("1ABC: add $1,$2,$3\n", &parsed, 8), "reject digit label");
    check(parsed.type == LINE_INVALID, "digit label invalid type");
    check(!parse_line("bad_label: add $1,$2,$3\n", &parsed, 9), "reject underscore label");
    check(!parse_line("add: .db 1\n", &parsed, 10), "reject opcode label");
}

static void test_invalid_unknown(void)
{
    ParsedLine parsed;

    check(!parse_line(".unknown 1\n", &parsed, 11), "reject unknown directive");
}

static void test_unknown_instruction_is_parsed(void)
{
    ParsedLine parsed;

    check(parse_line("unknown $1,$2\n", &parsed, 12), "parse unknown instruction name");
    check(parsed.type == LINE_INSTRUCTION, "unknown instruction type");
    check(strcmp(parsed.name, "unknown") == 0, "unknown instruction name");
    check(parsed.operand_count == 2, "unknown instruction operand count");
    check(strcmp(parsed.operands[0], "$1") == 0, "unknown instruction operand 1");
    check(strcmp(parsed.operands[1], "$2") == 0, "unknown instruction operand 2");
}

static void test_invalid_commas(void)
{
    ParsedLine parsed;

    check(!parse_line(".db ,1\n", &parsed, 13), "reject leading comma");
    check(!parse_line(".db 1,\n", &parsed, 14), "reject trailing comma");
    check(!parse_line(".db 1,,2\n", &parsed, 15), "reject double comma directive");
    check(!parse_line("add $1,, $2\n", &parsed, 16), "reject double comma instruction");
}

static void test_invalid_missing_after_label(void)
{
    ParsedLine parsed;

    check(!parse_line("LABEL:\n", &parsed, 17), "reject missing name after label");
}

static void test_invalid_long_line(void)
{
    ParsedLine parsed;
    char long_line[MAX_LINE_LENGTH + 4];
    int i;

    for (i = 0; i < MAX_LINE_LENGTH + 1; i++) {
        long_line[i] = 'a';
    }
    long_line[MAX_LINE_LENGTH + 1] = '\0';

    check(!parse_line(long_line, &parsed, 18), "reject long line");
}

int main(void)
{
    test_valid_empty_comment();
    test_valid_instruction();
    test_valid_directive_numbers();
    test_valid_asciz();
    test_valid_entry_extern();
    test_invalid_label();
    test_invalid_unknown();
    test_unknown_instruction_is_parsed();
    test_invalid_commas();
    test_invalid_missing_after_label();
    test_invalid_long_line();

    if (tests_failed != 0) {
        printf("%d parser tests failed\n", tests_failed);
        return 1;
    }

    printf("All parser tests passed\n");
    return 0;
}

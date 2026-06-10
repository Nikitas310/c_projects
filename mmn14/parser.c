#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"
#include "utils.h"
#include "errors.h"

static int is_directive_name(const char *name)
{
    return strcmp(name, ".db") == 0 ||
        strcmp(name, ".dh") == 0 ||
        strcmp(name, ".dw") == 0 ||
        strcmp(name, ".asciz") == 0 ||
        strcmp(name, ".entry") == 0 ||
        strcmp(name, ".extern") == 0;
}

static const char *skip_const_spaces(const char *str)
{
    while (*str != '\0' && isspace((unsigned char)*str)) {
        str++;
    }

    return str;
}

static void copy_token(
    const char **cursor,
    char *token
)
{
    int i;

    i = 0;
    while (**cursor != '\0' && !isspace((unsigned char)**cursor) && **cursor != ',') {
        token[i] = **cursor;
        (*cursor)++;
        i++;
    }
    token[i] = '\0';
}

static const char *find_label_colon(const char *cursor)
{
    while (*cursor != '\0' && !isspace((unsigned char)*cursor) && *cursor != ',') {
        if (*cursor == ':') {
            return cursor;
        }
        cursor++;
    }

    return NULL;
}

static void remove_newline(char *str)
{
    int length;

    length = (int)strlen(str);
    if (length > 0 && str[length - 1] == '\n') {
        str[length - 1] = '\0';
    }
}

static int line_has_valid_length(const char *line)
{
    int i;

    i = 0;
    while (line[i] != '\0' && line[i] != '\n') {
        i++;
    }

    return i <= MAX_LINE_LENGTH;
}

static int fail_parse(
    ParsedLine *parsed,
    int line_number,
    ErrorCode code,
    const char *details
)
{
    parsed->type = LINE_INVALID;
    report_error(line_number, code, details);
    return 0;
}

static int copy_operand(
    ParsedLine *parsed,
    int operand_index,
    const char *start,
    int length
)
{
    if (operand_index >= MAX_OPERANDS || length <= 0 || length > MAX_LINE_LENGTH) {
        return 0;
    }

    strncpy(parsed->operands[operand_index], start, length);
    parsed->operands[operand_index][length] = '\0';
    trim(parsed->operands[operand_index]);

    if (parsed->operands[operand_index][0] == '\0') {
        return 0;
    }

    return 1;
}

static int parse_asciz_operand(
    const char *cursor,
    ParsedLine *parsed,
    int line_number
)
{
    char operand[MAX_LINE_LENGTH + 1];

    cursor = skip_const_spaces(cursor);
    if (*cursor == '\0') {
        return fail_parse(parsed, line_number, ERROR_INTERNAL, "missing operand");
    }

    if (strlen(cursor) > MAX_LINE_LENGTH) {
        return fail_parse(parsed, line_number, ERROR_INTERNAL, "operand too long");
    }

    strcpy(operand, cursor);
    trim(operand);

    if (operand[0] == '\0') {
        return fail_parse(parsed, line_number, ERROR_INTERNAL, "missing operand");
    }

    strcpy(parsed->operands[0], operand);
    parsed->operand_count = 1;
    return 1;
}

static int parse_operands(
    const char *cursor,
    ParsedLine *parsed,
    int line_number
)
{
    const char *operand_start;
    const char *operand_end;
    int expect_operand;

    cursor = skip_const_spaces(cursor);
    if (*cursor == '\0') {
        return 1;
    }

    if (strcmp(parsed->name, ".asciz") == 0) {
        return parse_asciz_operand(cursor, parsed, line_number);
    }

    expect_operand = 1;
    while (*cursor != '\0') {
        cursor = skip_const_spaces(cursor);

        if (*cursor == ',') {
            return fail_parse(parsed, line_number, ERROR_INTERNAL, "missing operand before comma");
        }

        operand_start = cursor;
        while (*cursor != '\0' && *cursor != ',') {
            cursor++;
        }

        operand_end = cursor;
        while (operand_end > operand_start &&
            isspace((unsigned char)*(operand_end - 1))) {
            operand_end--;
        }

        if (!copy_operand(
            parsed,
            parsed->operand_count,
            operand_start,
            (int)(operand_end - operand_start)
        )) {
            return fail_parse(parsed, line_number, ERROR_INTERNAL, "invalid operand");
        }
        parsed->operand_count++;
        expect_operand = 0;

        if (*cursor == ',') {
            cursor++;
            expect_operand = 1;
            if (*skip_const_spaces(cursor) == '\0') {
                return fail_parse(parsed, line_number, ERROR_INTERNAL, "trailing comma");
            }
        }
    }

    if (expect_operand) {
        return fail_parse(parsed, line_number, ERROR_INTERNAL, "missing operand");
    }

    return 1;
}

void clear_parsed_line(
    ParsedLine *parsed
)
{
    int i;

    if (parsed == NULL) {
        return;
    }

    parsed->type = LINE_EMPTY;
    parsed->has_label = 0;
    parsed->label[0] = '\0';
    parsed->name[0] = '\0';
    parsed->operand_count = 0;

    for (i = 0; i < MAX_OPERANDS; i++) {
        parsed->operands[i][0] = '\0';
    }
}

int parse_line(
    const char *line,
    ParsedLine *parsed,
    int line_number
)
{
    char work_line[MAX_LINE_LENGTH + 2];
    char token[MAX_LINE_LENGTH + 1];
    const char *colon;
    const char *cursor;

    if (parsed == NULL) {
        return 0;
    }

    clear_parsed_line(parsed);

    if (line == NULL) {
        return fail_parse(parsed, line_number, ERROR_INTERNAL, "null line");
    }

    if (!line_has_valid_length(line)) {
        return fail_parse(parsed, line_number, ERROR_INTERNAL, "line too long");
    }

    strcpy(work_line, line);
    remove_newline(work_line);

    cursor = skip_const_spaces(work_line);
    if (*cursor == '\0') {
        parsed->type = LINE_EMPTY;
        return 1;
    }

    if (*cursor == ';') {
        parsed->type = LINE_COMMENT;
        return 1;
    }

    colon = find_label_colon(cursor);
    if (colon != NULL) {
        if (colon == cursor || (colon - cursor) > MAX_LABEL_LENGTH) {
            return fail_parse(parsed, line_number, ERROR_INVALID_LABEL, cursor);
        }

        strncpy(parsed->label, cursor, colon - cursor);
        parsed->label[colon - cursor] = '\0';

        if (!is_valid_label(parsed->label)) {
            return fail_parse(parsed, line_number, ERROR_INVALID_LABEL, parsed->label);
        }

        parsed->has_label = 1;
        cursor = colon + 1;
    }

    cursor = skip_const_spaces(cursor);
    if (*cursor == '\0') {
        return fail_parse(parsed, line_number, ERROR_INTERNAL, "missing name after label");
    }

    copy_token(&cursor, token);
    if (token[0] == '\0') {
        return fail_parse(parsed, line_number, ERROR_INTERNAL, "missing statement name");
    }

    strcpy(parsed->name, token);

    if (parsed->name[0] == '.') {
        if (!is_directive_name(parsed->name)) {
            return fail_parse(parsed, line_number, ERROR_UNKNOWN_OPCODE, parsed->name);
        }
        parsed->type = LINE_DIRECTIVE;
    } else {
        parsed->type = LINE_INSTRUCTION;
    }

    return parse_operands(cursor, parsed, line_number);
}

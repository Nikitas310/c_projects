#ifndef PARSER_H
#define PARSER_H

#include "assembler.h"

typedef enum {
    LINE_EMPTY,
    LINE_COMMENT,
    LINE_DIRECTIVE,
    LINE_INSTRUCTION,
    LINE_INVALID
} LineType;

typedef struct {
    LineType type;

    int has_label;
    char label[MAX_LABEL_LENGTH + 1];

    char name[MAX_LINE_LENGTH + 1];

    char operands[MAX_OPERANDS][MAX_LINE_LENGTH + 1];
    int operand_count;
} ParsedLine;

void clear_parsed_line(
    ParsedLine *parsed
);

int parse_line(
    const char *line,
    ParsedLine *parsed,
    int line_number
);

#endif

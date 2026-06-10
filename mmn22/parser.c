#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "parser.h"

/* Execute parser function and immediately return if an error is detected. */
#define CHECK(x)                  \
    do {                          \
        status = (x);             \
        if (status != PARSE_OK)   \
            return status;        \
    } while (0)

/* Skip all whitespace characters starting from the current position. */
static void skip_spaces(char *line, int *i);

/* Check whether the current character marks the end of the input line. */
static int is_end(char c);

/* Check whether the current character marks the end of a token. */
static int is_token_end(char c);

/* Parse command name. */
static parse_status parse_cmd(char *line, int *i, parms *p);
/* Parse a complex variable name (A-F) and store the corresponding address. */
static parse_status parse_complex(char *line, int *i, complex **target);
/* Parse a numeric parameter. */
static parse_status parse_number(char *line, int *i, double *target);
/* Parse a mandatory comma separator. */
static parse_status parse_required_comma(char *line, int *i);
/* Verify that no extra text remains after the command has been parsed. */
static parse_status parse_end(char *line, int *i);

/* Initialize a parameter structure before parsing a new command. */
void clear_parms(parms *p)
{
    p->type = CMD_INVALID;
    p->comp1 = NULL;
    p->comp2 = NULL;
    p->real = 0;
    p->imag = 0;
}

/* Print the appropriate error message corresponding to a parser error code. */
void print_error(parse_status status)
{
    switch (status) {
        case ERR_UNKNOWN_COMMAND:
            printf("Undefined command name\n");
            break;

        case ERR_ILLEGAL_COMMA:
            printf("Illegal comma\n");
            break;

        case ERR_MISSING_COMMA:
            printf("Missing comma\n");
            break;

        case ERR_MISSING_PARAMETER:
            printf("Missing parameter\n");
            break;

        case ERR_UNDEFINED_COMPLEX:
            printf("Undefined complex variable\n");
            break;

        case ERR_EXTRANEOUS_TEXT:
            printf("Extraneous text after end of command\n");
            break;

        case ERR_INVALID_NUMBER:
            printf("Invalid parameter - not a number\n");
            break;

        case ERR_MULTIPLE_CONSECUTIVE_COMMAS:
            printf("Multiple consecutive commas\n");
            break;

        default:
            printf("Unknown error\n");
            break;
    }
}

/* Parse a complete command line and fill the parameter structure. */
parse_status parse_line(char *line, parms *p)
{
    int i = 0;
    parse_status status;

    CHECK(parse_cmd(line, &i, p));

    /* Parse command parameters according to the detected command type. */
    switch (p->type) {
        case CMD_READ_COMP:
            CHECK(parse_complex(line, &i, &p->comp1));
            CHECK(parse_required_comma(line, &i));
            CHECK(parse_number(line, &i, &p->real));
            CHECK(parse_required_comma(line, &i));
            CHECK(parse_number(line, &i, &p->imag));
            break;

        case CMD_PRINT_COMP:
            CHECK(parse_complex(line, &i, &p->comp1));
            break;

        case CMD_ADD_COMP:
        case CMD_SUB_COMP:
        case CMD_MULT_COMP_COMP:
            CHECK(parse_complex(line, &i, &p->comp1));
            CHECK(parse_required_comma(line, &i));
            CHECK(parse_complex(line, &i, &p->comp2));
            break;

        case CMD_MULT_COMP_REAL:
            CHECK(parse_complex(line, &i, &p->comp1));
            CHECK(parse_required_comma(line, &i));
            CHECK(parse_number(line, &i, &p->real));
            break;

        case CMD_MULT_COMP_IMG:
            CHECK(parse_complex(line, &i, &p->comp1));
            CHECK(parse_required_comma(line, &i));
            CHECK(parse_number(line, &i, &p->imag));
            break;

        case CMD_ABS_COMP:
            CHECK(parse_complex(line, &i, &p->comp1));
            break;

        case CMD_STOP:
            break;

        default:
            return ERR_UNKNOWN_COMMAND;
    }

    CHECK(parse_end(line, &i));

    return PARSE_OK;
}

static void skip_spaces(char *line, int *i)
{
    while (isspace((unsigned char)line[*i]))
    {
        (*i)++;
    }
}

static int is_end(char c)
{
    return c == '\0' || c == '\n';
}

static int is_token_end(char c)
{
    return c == ',' || isspace((unsigned char)c) || is_end(c);
}

/* Extract the command name and identify the corresponding command type. */
static parse_status parse_cmd(char *line, int *i, parms *p)
{
    int start;
    int len;
    int j;
    char token[MAX_TOKEN_LEN];

    skip_spaces(line, i);

    start = *i;

    while (!is_token_end(line[*i]))
        (*i)++;

    len = *i - start;

    if (len == 0) {
        p->type = CMD_INVALID;
        return ERR_UNKNOWN_COMMAND;
    }

    if (len >= MAX_TOKEN_LEN) {
        p->type = CMD_INVALID;
        return ERR_UNKNOWN_COMMAND;
    }

    strncpy(token, line + start, len);
    token[len] = '\0';

    /* Search command table */
    for (j = 0; cmd[j].type != CMD_INVALID; j++) {
        if (strcmp(token, cmd[j].name) == 0) {
            p->type = cmd[j].type;

            if (line[*i] == ',')
                return ERR_ILLEGAL_COMMA;

            return PARSE_OK;
        }
    }

    p->type = CMD_INVALID;
    return ERR_UNKNOWN_COMMAND;
}

/* Extract a complex variable name and resolve it to one of A-F. */
static parse_status parse_complex(char *line, int *i, complex **target)
{
    int start;
    int len;
    int j;

    skip_spaces(line, i);

    start = *i;

    while (!is_token_end(line[*i]))
        (*i)++;

    len = *i - start;

    /* Complex variable names must consist of a single letter */
    if (len == 0)
        return ERR_MISSING_PARAMETER;

    if (len != 1)
        return ERR_UNDEFINED_COMPLEX;

    for (j = 0; compNums[j].num != NULL; j++) {
        if (line[start] == compNums[j].name) {
            *target = compNums[j].num;
            return PARSE_OK;
        }
    }

    return ERR_UNDEFINED_COMPLEX;
}

static parse_status parse_number(char *line, int *i, double *target)
{
    int start;
    int len;
    int has_digit_before_dot = 0;
    int has_dot = 0;
    char number[MAX_TOKEN_LEN];

    skip_spaces(line, i);

    start = *i;

    if (line[*i] == ',')
        return ERR_MISSING_PARAMETER;

    if (is_end(line[*i]))
        return ERR_MISSING_PARAMETER;

    /* Optional leading sign */
    if (line[*i] == '+' || line[*i] == '-')
        (*i)++;

    /* Validate number format */
    while (!is_token_end(line[*i])) {
        if (isdigit((unsigned char)line[*i])) {
            if (!has_dot)
                has_digit_before_dot = 1;
        }
        else if (line[*i] == '.') {
            if (has_dot)
                return ERR_INVALID_NUMBER;

            if (!has_digit_before_dot)
                return ERR_INVALID_NUMBER;

            has_dot = 1;
        }
        else {
            return ERR_INVALID_NUMBER;
        }

        (*i)++;
    }

    if (!has_digit_before_dot)
        return ERR_INVALID_NUMBER;

    len = *i - start;

    if (len >= MAX_TOKEN_LEN)
        return ERR_INVALID_NUMBER;

    strncpy(number, line + start, len);
    number[len] = '\0';

    *target = atof(number);

    return PARSE_OK;
}

/* Parse a required comma between parameters and detect comma-related errors. */
static parse_status parse_required_comma(char *line, int *i)
{
    skip_spaces(line, i);

    if (is_end(line[*i]))
        return ERR_MISSING_PARAMETER;

    if (line[*i] != ',')
        return ERR_MISSING_COMMA;

    (*i)++;

    skip_spaces(line, i);

    if (line[*i] == ',')
        return ERR_MULTIPLE_CONSECUTIVE_COMMAS;

    if (is_end(line[*i]))
        return ERR_MISSING_PARAMETER;

    return PARSE_OK;
}

/* Ensure that the command ended correctly and no extra characters remain. */
static parse_status parse_end(char *line, int *i)
{
    skip_spaces(line, i);

    if (!is_end(line[*i]))
        return ERR_EXTRANEOUS_TEXT;

    return PARSE_OK;
}
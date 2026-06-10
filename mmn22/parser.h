#ifndef PARSER_H
#define PARSER_H

#include "complex.h"

#define MAX_TOKEN_LEN 64

/* Supported command types. */
typedef enum {
    CMD_READ_COMP,
    CMD_PRINT_COMP,
    CMD_ADD_COMP,
    CMD_SUB_COMP,
    CMD_MULT_COMP_REAL,
    CMD_MULT_COMP_IMG,
    CMD_MULT_COMP_COMP,
    CMD_ABS_COMP,
    CMD_STOP,
    CMD_INVALID
} cmd_type;

/* Parser status and error codes. */
typedef enum {
    PARSE_OK,
    ERR_UNKNOWN_COMMAND,
    ERR_ILLEGAL_COMMA,
    ERR_MISSING_COMMA,
    ERR_MISSING_PARAMETER,
    ERR_UNDEFINED_COMPLEX,
    ERR_EXTRANEOUS_TEXT,
    ERR_INVALID_NUMBER,
    ERR_MULTIPLE_CONSECUTIVE_COMMAS
} parse_status;

/* Parsed command parameters. */
typedef struct {
    cmd_type type;
    complex *comp1;
    complex *comp2;
    double real;
    double imag;
} parms;

/* Maps variable names (A-F) to complex number addresses. */
typedef struct {
    char name;
    complex *num;
} complex_store;

/* Maps command names to command types. */
typedef struct {
    char *name;
    cmd_type type;
} command_info;

/* Global command and variable tables. */
extern complex_store compNums[];
extern command_info cmd[];

/* Parse a complete command line. */
parse_status parse_line(char *line, parms *p);

/* Initialize parameter structure. */
void clear_parms(parms *p);

/* Print parser error message. */
void print_error(parse_status status);

#endif
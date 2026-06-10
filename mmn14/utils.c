#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include "assembler.h"
#include "opcode_table.h"
#include "utils.h"

static int is_directive_name(const char *name)
{
    return strcmp(name, ".db") == 0 ||
        strcmp(name, "db") == 0 ||
        strcmp(name, ".dh") == 0 ||
        strcmp(name, "dh") == 0 ||
        strcmp(name, ".dw") == 0 ||
        strcmp(name, "dw") == 0 ||
        strcmp(name, ".asciz") == 0 ||
        strcmp(name, "asciz") == 0 ||
        strcmp(name, ".entry") == 0 ||
        strcmp(name, "entry") == 0 ||
        strcmp(name, ".extern") == 0 ||
        strcmp(name, "extern") == 0;
}

static int is_macro_keyword(const char *name)
{
    return strcmp(name, "mcro") == 0 || strcmp(name, "mcroend") == 0;
}

void remove_trailing_spaces(char *str)
{
    int i;

    if (str == NULL) {
        return;
    }

    i = (int)strlen(str) - 1;
    while (i >= 0 && isspace((unsigned char)str[i])) {
        str[i] = '\0';
        i--;
    }
}

char *skip_spaces(char *str)
{
    if (str == NULL) {
        return NULL;
    }

    while (*str != '\0' && isspace((unsigned char)*str)) {
        str++;
    }

    return str;
}

void trim(char *str)
{
    char *start;

    if (str == NULL) {
        return;
    }

    remove_trailing_spaces(str);
    start = skip_spaces(str);

    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

int is_valid_label(
    const char *name
)
{
    int i;
    int length;

    if (name == NULL) {
        return 0;
    }

    length = (int)strlen(name);
    if (length == 0 || length > MAX_LABEL_LENGTH) {
        return 0;
    }

    if (!isalpha((unsigned char)name[0])) {
        return 0;
    }

    for (i = 1; i < length; i++) {
        if (!isalnum((unsigned char)name[i])) {
            return 0;
        }
    }

    if (find_opcode(name) != NULL || is_directive_name(name) || is_macro_keyword(name)) {
        return 0;
    }

    return 1;
}

int is_valid_register(
    const char *operand
)
{
    int value;

    return parse_register_number(operand, &value);
}

int parse_register_number(
    const char *operand,
    int *reg_number
)
{
    int value;

    if (operand == NULL || reg_number == NULL || operand[0] != '$') {
        return 0;
    }

    if (operand[1] == '\0') {
        return 0;
    }

    if (!isdigit((unsigned char)operand[1])) {
        return 0;
    }

    if (operand[2] != '\0' && !isdigit((unsigned char)operand[2])) {
        return 0;
    }

    if (operand[2] != '\0' && operand[3] != '\0') {
        return 0;
    }

    value = operand[1] - '0';
    if (operand[2] != '\0') {
        value = value * 10 + operand[2] - '0';
    }

    if (value < 0 || value > 31) {
        return 0;
    }

    *reg_number = value;
    return 1;
}

int is_valid_integer(
    const char *str
)
{
    long value;

    return parse_integer(str, &value);
}

int parse_integer(
    const char *str,
    long *value
)
{
    int i;
    char *endptr;

    if (str == NULL || value == NULL || str[0] == '\0') {
        return 0;
    }

    i = 0;
    if (str[i] == '+' || str[i] == '-') {
        i++;
    }

    if (str[i] == '\0') {
        return 0;
    }

    while (str[i] != '\0') {
        if (!isdigit((unsigned char)str[i])) {
            return 0;
        }
        i++;
    }

    errno = 0;
    *value = strtol(str, &endptr, 10);

    if (errno != 0 || *endptr != '\0') {
        return 0;
    }

    return 1;
}

void build_filename(
    const char *base_name,
    const char *extension,
    char *result
)
{
    strcpy(result, base_name);
    strcat(result, extension);
}

void process_file(const char *base_name)
{
    char filename[MAX_FILENAME_LENGTH];
    FILE *file;

    if (strlen(base_name) + strlen(AS_EXTENSION) >= MAX_FILENAME_LENGTH) {
        printf("Error: file name is too long %s%s\n", base_name, AS_EXTENSION);
        return;
    }

    build_filename(base_name, AS_EXTENSION, filename);

    file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: cannot open file %s\n", filename);
        return;
    }

    printf("Processing %s\n", filename);

    fclose(file);
}

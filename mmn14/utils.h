#ifndef UTILS_H
#define UTILS_H

void trim(char *str);

char *skip_spaces(char *str);

void remove_trailing_spaces(char *str);

int is_valid_label(
    const char *name
);

int is_valid_register(
    const char *operand
);

int parse_register_number(
    const char *operand,
    int *reg_number
);

int is_valid_integer(
    const char *str
);

int parse_integer(
    const char *str,
    long *value
);

void process_file(const char *base_name);

void build_filename(
    const char *base_name,
    const char *extension,
    char *result
);

#endif

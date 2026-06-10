#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "assembler.h"

#define SYMBOL_CODE      1
#define SYMBOL_DATA      2
#define SYMBOL_EXTERN    4
#define SYMBOL_ENTRY     8

typedef struct Symbol {
    char name[MAX_LABEL_LENGTH + 1];
    int address;
    unsigned int attributes;
    struct Symbol *next;
} Symbol;

Symbol *find_symbol(
    Symbol *head,
    const char *name
);

int add_symbol(
    Symbol **head,
    const char *name,
    int address,
    unsigned int attributes
);

int symbol_exists(
    Symbol *head,
    const char *name
);

void update_data_symbols(
    Symbol *head,
    int icf
);

void free_symbol_table(
    Symbol *head
);

#endif

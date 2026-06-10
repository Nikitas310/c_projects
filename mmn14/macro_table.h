#ifndef MACRO_TABLE_H
#define MACRO_TABLE_H

#include "assembler.h"

typedef struct MacroLine {
    char *text;
    struct MacroLine *next;
} MacroLine;

typedef struct Macro {
    char name[MAX_MACRO_NAME_LENGTH + 1];
    MacroLine *lines;
    struct Macro *next;
} Macro;

Macro *find_macro(
    Macro *head,
    const char *name
);

int add_macro(
    Macro **head,
    const char *name
);

int add_line_to_macro(
    Macro *macro,
    const char *line
);

void free_macro_table(
    Macro *head
);

#endif

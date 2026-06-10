#include <stdlib.h>
#include <string.h>
#include "macro_table.h"

static char *duplicate_string(const char *str)
{
    char *copy;

    copy = (char *)malloc(strlen(str) + 1);
    if (copy == NULL) {
        return NULL;
    }

    strcpy(copy, str);
    return copy;
}

Macro *find_macro(
    Macro *head,
    const char *name
)
{
    while (head != NULL) {
        if (strcmp(head->name, name) == 0) {
            return head;
        }
        head = head->next;
    }

    return NULL;
}

int add_macro(
    Macro **head,
    const char *name
)
{
    Macro *new_macro;

    if (head == NULL || name == NULL || find_macro(*head, name) != NULL) {
        return 0;
    }

    if (strlen(name) > MAX_MACRO_NAME_LENGTH) {
        return 0;
    }

    new_macro = (Macro *)malloc(sizeof(Macro));
    if (new_macro == NULL) {
        return 0;
    }

    strcpy(new_macro->name, name);
    new_macro->lines = NULL;
    new_macro->next = *head;
    *head = new_macro;

    return 1;
}

int add_line_to_macro(
    Macro *macro,
    const char *line
)
{
    MacroLine *new_line;
    MacroLine *current;

    if (macro == NULL || line == NULL) {
        return 0;
    }

    new_line = (MacroLine *)malloc(sizeof(MacroLine));
    if (new_line == NULL) {
        return 0;
    }

    new_line->text = duplicate_string(line);
    if (new_line->text == NULL) {
        free(new_line);
        return 0;
    }

    new_line->next = NULL;

    if (macro->lines == NULL) {
        macro->lines = new_line;
        return 1;
    }

    current = macro->lines;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = new_line;

    return 1;
}

void free_macro_table(
    Macro *head
)
{
    Macro *next_macro;
    MacroLine *line;
    MacroLine *next_line;

    while (head != NULL) {
        next_macro = head->next;
        line = head->lines;

        while (line != NULL) {
            next_line = line->next;
            free(line->text);
            free(line);
            line = next_line;
        }

        free(head);
        head = next_macro;
    }
}

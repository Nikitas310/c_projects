#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"

Symbol *find_symbol(
    Symbol *head,
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

int add_symbol(
    Symbol **head,
    const char *name,
    int address,
    unsigned int attributes
)
{
    Symbol *new_symbol;

    if (head == NULL || name == NULL || symbol_exists(*head, name)) {
        return 0;
    }

    if (strlen(name) > MAX_LABEL_LENGTH) {
        return 0;
    }

    new_symbol = (Symbol *)malloc(sizeof(Symbol));
    if (new_symbol == NULL) {
        return 0;
    }

    strcpy(new_symbol->name, name);
    new_symbol->address = address;
    new_symbol->attributes = attributes;
    new_symbol->next = *head;
    *head = new_symbol;

    return 1;
}

int symbol_exists(
    Symbol *head,
    const char *name
)
{
    return find_symbol(head, name) != NULL;
}

void update_data_symbols(
    Symbol *head,
    int icf
)
{
    while (head != NULL) {
        if ((head->attributes & SYMBOL_DATA) != 0 &&
            (head->attributes & SYMBOL_CODE) == 0 &&
            (head->attributes & SYMBOL_EXTERN) == 0) {
            head->address += icf;
        }
        head = head->next;
    }
}

void free_symbol_table(
    Symbol *head
)
{
    Symbol *next;

    while (head != NULL) {
        next = head->next;
        free(head);
        head = next;
    }
}

#include <stdlib.h>
#include <string.h>
#include "external_usage.h"

int add_external_usage(
    ExternalUsage **head,
    const char *name,
    int address
)
{
    ExternalUsage *new_usage;
    ExternalUsage *current;

    if (head == 0 || name == 0 || strlen(name) > MAX_LABEL_LENGTH) {
        return 0;
    }

    new_usage = (ExternalUsage *)malloc(sizeof(ExternalUsage));
    if (new_usage == 0) {
        return 0;
    }

    strcpy(new_usage->name, name);
    new_usage->address = address;
    new_usage->next = 0;

    if (*head == 0) {
        *head = new_usage;
        return 1;
    }

    current = *head;
    while (current->next != 0) {
        current = current->next;
    }
    current->next = new_usage;

    return 1;
}

ExternalUsage *find_external_usage(
    ExternalUsage *head,
    const char *name,
    int address
)
{
    while (head != 0) {
        if (head->address == address && strcmp(head->name, name) == 0) {
            return head;
        }
        head = head->next;
    }

    return 0;
}

int count_external_usages(
    ExternalUsage *head
)
{
    int count;

    count = 0;
    while (head != 0) {
        count++;
        head = head->next;
    }

    return count;
}

void free_external_usages(
    ExternalUsage *head
)
{
    ExternalUsage *next;

    while (head != 0) {
        next = head->next;
        free(head);
        head = next;
    }
}

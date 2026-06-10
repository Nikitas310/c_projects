#ifndef EXTERNAL_USAGE_H
#define EXTERNAL_USAGE_H

#include "assembler.h"

typedef struct ExternalUsage {
    char name[MAX_LABEL_LENGTH + 1];
    int address;
    struct ExternalUsage *next;
} ExternalUsage;

int add_external_usage(
    ExternalUsage **head,
    const char *name,
    int address
);

ExternalUsage *find_external_usage(
    ExternalUsage *head,
    const char *name,
    int address
);

int count_external_usages(
    ExternalUsage *head
);

void free_external_usages(
    ExternalUsage *head
);

#endif

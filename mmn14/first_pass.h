#ifndef FIRST_PASS_H
#define FIRST_PASS_H

#include "symbol_table.h"
#include "data_image.h"
#include "code_image.h"
#include "external_usage.h"

typedef struct {
    Symbol *symbols;

    DataImage data_image;
    CodeImage code_image;

    int ic;
    int dc;

    int icf;
    int dcf;

    ExternalUsage *external_usages;

    int has_errors;
} FirstPassContext;

void init_first_pass_context(
    FirstPassContext *context
);

void free_first_pass_context(
    FirstPassContext *context
);

int run_first_pass_directives_only(
    const char *base_name,
    FirstPassContext *context
);

int run_first_pass(
    const char *base_name,
    FirstPassContext *context
);

#endif

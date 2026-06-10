#ifndef CODE_IMAGE_H
#define CODE_IMAGE_H

#include "assembler.h"

typedef enum {
    UNRESOLVED_NONE,
    UNRESOLVED_BRANCH_LABEL,
    UNRESOLVED_J_LABEL
} UnresolvedType;

typedef struct {
    int address;
    unsigned int word;

    UnresolvedType unresolved_type;
    char unresolved_label[MAX_LABEL_LENGTH + 1];
} CodeWord;

typedef struct {
    CodeWord items[MAX_CODE_IMAGE_SIZE];
    int count;
} CodeImage;

void init_code_image(
    CodeImage *image
);

int add_code_word(
    CodeImage *image,
    int address,
    unsigned int word,
    UnresolvedType unresolved_type,
    const char *unresolved_label
);

CodeWord *find_code_word_by_address(
    CodeImage *image,
    int address
);

void clear_code_image(
    CodeImage *image
);

#endif

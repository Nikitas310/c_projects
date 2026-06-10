#include <string.h>
#include "code_image.h"

void init_code_image(
    CodeImage *image
)
{
    if (image != 0) {
        image->count = 0;
    }
}

int add_code_word(
    CodeImage *image,
    int address,
    unsigned int word,
    UnresolvedType unresolved_type,
    const char *unresolved_label
)
{
    if (image == 0 || image->count >= MAX_CODE_IMAGE_SIZE) {
        return 0;
    }

    image->items[image->count].address = address;
    image->items[image->count].word = word;
    image->items[image->count].unresolved_type = unresolved_type;
    image->items[image->count].unresolved_label[0] = '\0';

    if (unresolved_label != 0) {
        strncpy(
            image->items[image->count].unresolved_label,
            unresolved_label,
            MAX_LABEL_LENGTH
        );
        image->items[image->count].unresolved_label[MAX_LABEL_LENGTH] = '\0';
    }

    image->count++;
    return 1;
}

CodeWord *find_code_word_by_address(
    CodeImage *image,
    int address
)
{
    int i;

    if (image == 0) {
        return 0;
    }

    for (i = 0; i < image->count; i++) {
        if (image->items[i].address == address) {
            return &image->items[i];
        }
    }

    return 0;
}

int has_unresolved_code_words(
    CodeImage *image
)
{
    int i;

    if (image == 0) {
        return 0;
    }

    for (i = 0; i < image->count; i++) {
        if (image->items[i].unresolved_type != UNRESOLVED_NONE) {
            return 1;
        }
    }

    return 0;
}

void clear_code_image(
    CodeImage *image
)
{
    if (image != 0) {
        image->count = 0;
    }
}

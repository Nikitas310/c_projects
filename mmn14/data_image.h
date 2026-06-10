#ifndef DATA_IMAGE_H
#define DATA_IMAGE_H

#include "assembler.h"

typedef struct {
    int address;
    unsigned char value;
} DataByte;

typedef struct {
    DataByte items[MAX_DATA_IMAGE_SIZE];
    int count;
} DataImage;

void init_data_image(
    DataImage *image
);

int add_data_byte(
    DataImage *image,
    int address,
    unsigned char value
);

void update_data_image_addresses(
    DataImage *image,
    int icf
);

void clear_data_image(
    DataImage *image
);

#endif

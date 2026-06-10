#include "data_image.h"

void init_data_image(
    DataImage *image
)
{
    if (image != 0) {
        image->count = 0;
    }
}

int add_data_byte(
    DataImage *image,
    int address,
    unsigned char value
)
{
    if (image == 0 || image->count >= MAX_DATA_IMAGE_SIZE) {
        return 0;
    }

    image->items[image->count].address = address;
    image->items[image->count].value = value;
    image->count++;

    return 1;
}

void update_data_image_addresses(
    DataImage *image,
    int icf
)
{
    int i;

    if (image == 0) {
        return;
    }

    for (i = 0; i < image->count; i++) {
        image->items[i].address += icf;
    }
}

void clear_data_image(
    DataImage *image
)
{
    if (image != 0) {
        image->count = 0;
    }
}

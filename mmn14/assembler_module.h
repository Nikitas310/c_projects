#ifndef ASSEMBLER_MODULE_H
#define ASSEMBLER_MODULE_H

int assemble_file(
    const char *base_name
);

int get_base_name_from_as_filename(
    const char *input_filename,
    char *base_name
);

#endif

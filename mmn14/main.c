#include <stdio.h>
#include "assembler.h"
#include "assembler_module.h"

int main(int argc, char *argv[])
{
    int i;
    char base_name[MAX_FILENAME_LENGTH];

    if (argc < 2) {
        printf("Usage: assembler <file1.as> <file2.as> ...\n");
        return 1;
    }

    for (i = 1; i < argc; i++) {
        if (!get_base_name_from_as_filename(argv[i], base_name)) {
            printf("Error: invalid input file name %s\n", argv[i]);
            continue;
        }
        assemble_file(base_name);
    }

    return 0;
}

#include <stddef.h>
#include <string.h>
#include "opcode_table.h"

static OpcodeInfo opcode_table[] = {
    {"add", 0, 1, 'R'},
    {"sub", 0, 2, 'R'},
    {"and", 0, 3, 'R'},
    {"or", 0, 4, 'R'},
    {"nor", 0, 5, 'R'},
    {"move", 1, 1, 'R'},
    {"mvhi", 1, 2, 'R'},
    {"mvlo", 1, 3, 'R'},
    {"addi", 10, 0, 'I'},
    {"subi", 11, 0, 'I'},
    {"andi", 12, 0, 'I'},
    {"ori", 13, 0, 'I'},
    {"nori", 14, 0, 'I'},
    {"bne", 15, 0, 'I'},
    {"beq", 16, 0, 'I'},
    {"blt", 17, 0, 'I'},
    {"bgt", 18, 0, 'I'},
    {"lb", 19, 0, 'I'},
    {"sb", 20, 0, 'I'},
    {"lw", 21, 0, 'I'},
    {"sw", 22, 0, 'I'},
    {"lh", 23, 0, 'I'},
    {"sh", 24, 0, 'I'},
    {"jmp", 30, 0, 'J'},
    {"la", 31, 0, 'J'},
    {"call", 32, 0, 'J'},
    {"hlt", 63, 0, 'J'},
    {NULL, 0, 0, '\0'}
};

OpcodeInfo *find_opcode(
    const char *name
)
{
    int i;

    if (name == NULL) {
        return NULL;
    }

    for (i = 0; opcode_table[i].name != NULL; i++) {
        if (strcmp(opcode_table[i].name, name) == 0) {
            return &opcode_table[i];
        }
    }

    return NULL;
}

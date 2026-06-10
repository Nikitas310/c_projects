#ifndef OPCODE_TABLE_H
#define OPCODE_TABLE_H

typedef struct {
    const char *name;
    int opcode;
    int funct;
    char type;
} OpcodeInfo;

OpcodeInfo *find_opcode(
    const char *name
);

#endif

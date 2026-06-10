#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#define MAX_LABEL_LENGTH      31
#define MAX_MACRO_NAME_LENGTH 31

#define MAX_FILENAME_LENGTH   256

#define MAX_LINE_LENGTH       80
#define MAX_OPERANDS          10

#define INITIAL_IC            100
#define INITIAL_DC            0
#define MAX_DATA_IMAGE_SIZE   4096
#define MAX_CODE_IMAGE_SIZE   4096

#define INSTRUCTION_SIZE      4

#define R_UNUSED_BITS         0
#define J_REGISTER_MODE       1
#define J_LABEL_MODE          0

#define AS_EXTENSION  ".as"
#define AM_EXTENSION  ".am"
#define OB_EXTENSION  ".ob"
#define ENT_EXTENSION ".ent"
#define EXT_EXTENSION ".ext"

#endif

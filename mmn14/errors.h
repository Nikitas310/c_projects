#ifndef ERRORS_H
#define ERRORS_H

typedef enum {
    ERROR_NONE,

    ERROR_DUPLICATE_LABEL,
    ERROR_INVALID_LABEL,
    ERROR_INVALID_REGISTER,
    ERROR_INVALID_NUMBER,
    ERROR_UNKNOWN_OPCODE,
    ERROR_MEMORY_ALLOCATION,

    ERROR_INTERNAL
} ErrorCode;

void report_error(
    int line_number,
    ErrorCode code,
    const char *details
);

#endif

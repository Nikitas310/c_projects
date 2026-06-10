#include <stdio.h>
#include "errors.h"

static const char *get_error_message(ErrorCode code)
{
    switch (code) {
    case ERROR_NONE:
        return "No error";
    case ERROR_DUPLICATE_LABEL:
        return "Duplicate label";
    case ERROR_INVALID_LABEL:
        return "Invalid label";
    case ERROR_INVALID_REGISTER:
        return "Invalid register";
    case ERROR_INVALID_NUMBER:
        return "Invalid number";
    case ERROR_UNKNOWN_OPCODE:
        return "Unknown opcode";
    case ERROR_MEMORY_ALLOCATION:
        return "Memory allocation failed";
    case ERROR_INTERNAL:
        return "Internal error";
    default:
        return "Internal error";
    }
}

void report_error(
    int line_number,
    ErrorCode code,
    const char *details
)
{
    printf("Line %d: %s", line_number, get_error_message(code));

    if (details != NULL && details[0] != '\0') {
        printf(" (%s)", details);
    }

    printf("\n");
}

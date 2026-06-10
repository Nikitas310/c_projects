#include <stdio.h>
#include <string.h>
#include "assembler.h"
#include "first_pass.h"
#include "parser.h"
#include "utils.h"
#include "errors.h"
#include "opcode_table.h"

static unsigned int encode_r(
    int opcode,
    int rs,
    int rt,
    int rd,
    int funct
)
{
    return ((unsigned int)opcode << 26) |
        ((unsigned int)rs << 21) |
        ((unsigned int)rt << 16) |
        ((unsigned int)rd << 11) |
        ((unsigned int)funct << 6) |
        R_UNUSED_BITS;
}

static unsigned int encode_i(
    int opcode,
    int rs,
    int rt,
    int immed
)
{
    return ((unsigned int)opcode << 26) |
        ((unsigned int)rs << 21) |
        ((unsigned int)rt << 16) |
        ((unsigned int)(immed & 0xffff));
}

static unsigned int encode_j(
    int opcode,
    int reg,
    int address
)
{
    return ((unsigned int)opcode << 26) |
        ((unsigned int)reg << 25) |
        ((unsigned int)address & 0x1ffffff);
}

static int is_r_arithmetic_name(const char *name)
{
    return strcmp(name, "add") == 0 ||
        strcmp(name, "sub") == 0 ||
        strcmp(name, "and") == 0 ||
        strcmp(name, "or") == 0 ||
        strcmp(name, "nor") == 0;
}

static int is_r_copy_name(const char *name)
{
    return strcmp(name, "move") == 0 ||
        strcmp(name, "mvhi") == 0 ||
        strcmp(name, "mvlo") == 0;
}

static int is_i_immediate_name(const char *name)
{
    return strcmp(name, "addi") == 0 ||
        strcmp(name, "subi") == 0 ||
        strcmp(name, "andi") == 0 ||
        strcmp(name, "ori") == 0 ||
        strcmp(name, "nori") == 0 ||
        strcmp(name, "lb") == 0 ||
        strcmp(name, "sb") == 0 ||
        strcmp(name, "lw") == 0 ||
        strcmp(name, "sw") == 0 ||
        strcmp(name, "lh") == 0 ||
        strcmp(name, "sh") == 0;
}

static int is_i_branch_name(const char *name)
{
    return strcmp(name, "beq") == 0 ||
        strcmp(name, "bne") == 0 ||
        strcmp(name, "blt") == 0 ||
        strcmp(name, "bgt") == 0;
}

static void mark_error(
    FirstPassContext *context,
    int line_number,
    ErrorCode code,
    const char *details
)
{
    report_error(line_number, code, details);
    context->has_errors = 1;
}

static int add_data_label(
    FirstPassContext *context,
    ParsedLine *parsed,
    int line_number
)
{
    if (!parsed->has_label) {
        return 1;
    }

    if (symbol_exists(context->symbols, parsed->label)) {
        report_error(line_number, ERROR_DUPLICATE_LABEL, parsed->label);
        context->has_errors = 1;
        return 0;
    }

    if (!add_symbol(&context->symbols, parsed->label, context->dc, SYMBOL_DATA)) {
        report_error(line_number, ERROR_MEMORY_ALLOCATION, parsed->label);
        context->has_errors = 1;
        return 0;
    }

    return 1;
}

static int add_byte_to_context(
    FirstPassContext *context,
    int line_number,
    unsigned char value
)
{
    if (!add_data_byte(&context->data_image, context->dc, value)) {
        report_error(line_number, ERROR_INTERNAL, "data image overflow");
        context->has_errors = 1;
        return 0;
    }

    context->dc++;
    return 1;
}

static int write_integer_bytes(
    FirstPassContext *context,
    int line_number,
    long value,
    int byte_count
)
{
    int i;
    unsigned long raw_value;

    raw_value = (unsigned long)value;
    for (i = 0; i < byte_count; i++) {
        if (!add_byte_to_context(
            context,
            line_number,
            (unsigned char)((raw_value >> (8 * i)) & 0xff)
        )) {
            return 0;
        }
    }

    return 1;
}

static int operand_count_is_at_least_one(
    FirstPassContext *context,
    ParsedLine *parsed,
    int line_number
)
{
    if (parsed->operand_count == 0) {
        report_error(line_number, ERROR_INTERNAL, "missing operands");
        context->has_errors = 1;
        return 0;
    }

    return 1;
}

static int process_integer_directive(
    FirstPassContext *context,
    ParsedLine *parsed,
    int line_number,
    long min_value,
    long max_value,
    int byte_count
)
{
    int i;
    long value;

    if (!operand_count_is_at_least_one(context, parsed, line_number)) {
        return 0;
    }

    if (!add_data_label(context, parsed, line_number)) {
        return 0;
    }

    for (i = 0; i < parsed->operand_count; i++) {
        if (!parse_integer(parsed->operands[i], &value)) {
            report_error(line_number, ERROR_INVALID_NUMBER, parsed->operands[i]);
            context->has_errors = 1;
            return 0;
        }

        if (value < min_value || value > max_value) {
            report_error(line_number, ERROR_INVALID_NUMBER, parsed->operands[i]);
            context->has_errors = 1;
            return 0;
        }

        if (!write_integer_bytes(context, line_number, value, byte_count)) {
            return 0;
        }
    }

    return 1;
}

static int process_asciz_directive(
    FirstPassContext *context,
    ParsedLine *parsed,
    int line_number
)
{
    char *operand;
    int length;
    int i;

    if (parsed->operand_count != 1) {
        report_error(line_number, ERROR_INTERNAL, "invalid asciz operand count");
        context->has_errors = 1;
        return 0;
    }

    operand = parsed->operands[0];
    length = (int)strlen(operand);
    if (length < 2 || operand[0] != '"' || operand[length - 1] != '"') {
        report_error(line_number, ERROR_INTERNAL, "invalid asciz string");
        context->has_errors = 1;
        return 0;
    }

    if (!add_data_label(context, parsed, line_number)) {
        return 0;
    }

    for (i = 1; i < length - 1; i++) {
        if (!add_byte_to_context(context, line_number, (unsigned char)operand[i])) {
            return 0;
        }
    }

    return add_byte_to_context(context, line_number, 0);
}

static int validate_single_label_operand(
    FirstPassContext *context,
    ParsedLine *parsed,
    int line_number
)
{
    if (parsed->operand_count != 1) {
        report_error(line_number, ERROR_INTERNAL, "invalid operand count");
        context->has_errors = 1;
        return 0;
    }

    if (!is_valid_label(parsed->operands[0])) {
        report_error(line_number, ERROR_INVALID_LABEL, parsed->operands[0]);
        context->has_errors = 1;
        return 0;
    }

    return 1;
}

static int process_extern_directive(
    FirstPassContext *context,
    ParsedLine *parsed,
    int line_number
)
{
    Symbol *symbol;

    if (!validate_single_label_operand(context, parsed, line_number)) {
        return 0;
    }

    symbol = find_symbol(context->symbols, parsed->operands[0]);
    if (symbol != 0) {
        if ((symbol->attributes & SYMBOL_EXTERN) != 0) {
            return 1;
        }

        report_error(line_number, ERROR_DUPLICATE_LABEL, parsed->operands[0]);
        context->has_errors = 1;
        return 0;
    }

    if (!add_symbol(&context->symbols, parsed->operands[0], 0, SYMBOL_EXTERN)) {
        report_error(line_number, ERROR_MEMORY_ALLOCATION, parsed->operands[0]);
        context->has_errors = 1;
        return 0;
    }

    return 1;
}

static int process_entry_directive(
    FirstPassContext *context,
    ParsedLine *parsed,
    int line_number
)
{
    return validate_single_label_operand(context, parsed, line_number);
}

static void process_directive(
    FirstPassContext *context,
    ParsedLine *parsed,
    int line_number
)
{
    if (strcmp(parsed->name, ".db") == 0) {
        process_integer_directive(context, parsed, line_number, -128, 127, 1);
    } else if (strcmp(parsed->name, ".dh") == 0) {
        process_integer_directive(context, parsed, line_number, -32768, 32767, 2);
    } else if (strcmp(parsed->name, ".dw") == 0) {
        process_integer_directive(
            context,
            parsed,
            line_number,
            -2147483647L - 1L,
            2147483647L,
            4
        );
    } else if (strcmp(parsed->name, ".asciz") == 0) {
        process_asciz_directive(context, parsed, line_number);
    } else if (strcmp(parsed->name, ".extern") == 0) {
        process_extern_directive(context, parsed, line_number);
    } else if (strcmp(parsed->name, ".entry") == 0) {
        process_entry_directive(context, parsed, line_number);
    }
}

static int add_code_label(
    FirstPassContext *context,
    ParsedLine *parsed,
    int line_number
)
{
    if (!parsed->has_label) {
        return 1;
    }

    if (symbol_exists(context->symbols, parsed->label)) {
        mark_error(context, line_number, ERROR_DUPLICATE_LABEL, parsed->label);
        return 0;
    }

    if (!add_symbol(&context->symbols, parsed->label, context->ic, SYMBOL_CODE)) {
        mark_error(context, line_number, ERROR_MEMORY_ALLOCATION, parsed->label);
        return 0;
    }

    return 1;
}

static int validate_operand_count(
    FirstPassContext *context,
    ParsedLine *parsed,
    int line_number,
    int expected
)
{
    if (parsed->operand_count != expected) {
        mark_error(context, line_number, ERROR_INTERNAL, "wrong operand count");
        return 0;
    }

    return 1;
}

static int parse_immediate_16(
    FirstPassContext *context,
    const char *operand,
    int line_number,
    int *value
)
{
    long parsed_value;

    if (!parse_integer(operand, &parsed_value)) {
        mark_error(context, line_number, ERROR_INVALID_NUMBER, operand);
        return 0;
    }

    if (parsed_value < -32768 || parsed_value > 32767) {
        mark_error(context, line_number, ERROR_INVALID_NUMBER, operand);
        return 0;
    }

    *value = (int)parsed_value;
    return 1;
}

static int parse_register_operand(
    FirstPassContext *context,
    const char *operand,
    int line_number,
    int *reg
)
{
    if (!parse_register_number(operand, reg)) {
        mark_error(context, line_number, ERROR_INVALID_REGISTER, operand);
        return 0;
    }

    return 1;
}

static int add_instruction_word(
    FirstPassContext *context,
    int line_number,
    unsigned int word,
    UnresolvedType unresolved_type,
    const char *unresolved_label
)
{
    if (!add_code_word(
        &context->code_image,
        context->ic,
        word,
        unresolved_type,
        unresolved_label
    )) {
        mark_error(context, line_number, ERROR_INTERNAL, "code image overflow");
        return 0;
    }

    return 1;
}

static int process_r_arithmetic(
    FirstPassContext *context,
    ParsedLine *parsed,
    OpcodeInfo *opcode,
    int line_number
)
{
    int rs;
    int rt;
    int rd;
    unsigned int word;

    if (!validate_operand_count(context, parsed, line_number, 3)) {
        return 0;
    }

    if (!parse_register_operand(context, parsed->operands[0], line_number, &rs) ||
        !parse_register_operand(context, parsed->operands[1], line_number, &rt) ||
        !parse_register_operand(context, parsed->operands[2], line_number, &rd)) {
        return 0;
    }

    word = encode_r(opcode->opcode, rs, rt, rd, opcode->funct);
    return add_instruction_word(context, line_number, word, UNRESOLVED_NONE, 0);
}

static int process_r_copy(
    FirstPassContext *context,
    ParsedLine *parsed,
    OpcodeInfo *opcode,
    int line_number
)
{
    int rs;
    int rd;
    unsigned int word;

    if (!validate_operand_count(context, parsed, line_number, 2)) {
        return 0;
    }

    if (!parse_register_operand(context, parsed->operands[0], line_number, &rs) ||
        !parse_register_operand(context, parsed->operands[1], line_number, &rd)) {
        return 0;
    }

    word = encode_r(opcode->opcode, rs, 0, rd, opcode->funct);
    return add_instruction_word(context, line_number, word, UNRESOLVED_NONE, 0);
}

static int process_i_immediate(
    FirstPassContext *context,
    ParsedLine *parsed,
    OpcodeInfo *opcode,
    int line_number
)
{
    int rs;
    int rt;
    int immed;
    unsigned int word;

    if (!validate_operand_count(context, parsed, line_number, 3)) {
        return 0;
    }

    if (!parse_register_operand(context, parsed->operands[0], line_number, &rs) ||
        !parse_immediate_16(context, parsed->operands[1], line_number, &immed) ||
        !parse_register_operand(context, parsed->operands[2], line_number, &rt)) {
        return 0;
    }

    word = encode_i(opcode->opcode, rs, rt, immed);
    return add_instruction_word(context, line_number, word, UNRESOLVED_NONE, 0);
}

static int process_i_branch(
    FirstPassContext *context,
    ParsedLine *parsed,
    OpcodeInfo *opcode,
    int line_number
)
{
    int rs;
    int rt;
    unsigned int word;

    if (!validate_operand_count(context, parsed, line_number, 3)) {
        return 0;
    }

    if (!parse_register_operand(context, parsed->operands[0], line_number, &rs) ||
        !parse_register_operand(context, parsed->operands[1], line_number, &rt)) {
        return 0;
    }

    if (!is_valid_label(parsed->operands[2])) {
        mark_error(context, line_number, ERROR_INVALID_LABEL, parsed->operands[2]);
        return 0;
    }

    word = encode_i(opcode->opcode, rs, rt, 0);
    return add_instruction_word(
        context,
        line_number,
        word,
        UNRESOLVED_BRANCH_LABEL,
        parsed->operands[2]
    );
}

static int process_jmp(
    FirstPassContext *context,
    ParsedLine *parsed,
    OpcodeInfo *opcode,
    int line_number
)
{
    int reg;
    unsigned int word;

    if (!validate_operand_count(context, parsed, line_number, 1)) {
        return 0;
    }

    if (parse_register_number(parsed->operands[0], &reg)) {
        word = encode_j(opcode->opcode, J_REGISTER_MODE, reg);
        return add_instruction_word(context, line_number, word, UNRESOLVED_NONE, 0);
    }

    if (!is_valid_label(parsed->operands[0])) {
        mark_error(context, line_number, ERROR_INVALID_LABEL, parsed->operands[0]);
        return 0;
    }

    word = encode_j(opcode->opcode, J_LABEL_MODE, 0);
    return add_instruction_word(
        context,
        line_number,
        word,
        UNRESOLVED_J_LABEL,
        parsed->operands[0]
    );
}

static int process_j_label(
    FirstPassContext *context,
    ParsedLine *parsed,
    OpcodeInfo *opcode,
    int line_number
)
{
    unsigned int word;

    if (!validate_operand_count(context, parsed, line_number, 1)) {
        return 0;
    }

    if (is_valid_register(parsed->operands[0]) || !is_valid_label(parsed->operands[0])) {
        mark_error(context, line_number, ERROR_INVALID_LABEL, parsed->operands[0]);
        return 0;
    }

    word = encode_j(opcode->opcode, J_LABEL_MODE, 0);
    return add_instruction_word(
        context,
        line_number,
        word,
        UNRESOLVED_J_LABEL,
        parsed->operands[0]
    );
}

static int process_hlt(
    FirstPassContext *context,
    ParsedLine *parsed,
    OpcodeInfo *opcode,
    int line_number
)
{
    unsigned int word;

    if (!validate_operand_count(context, parsed, line_number, 0)) {
        return 0;
    }

    word = encode_j(opcode->opcode, J_LABEL_MODE, 0);
    return add_instruction_word(context, line_number, word, UNRESOLVED_NONE, 0);
}

static void process_instruction(
    FirstPassContext *context,
    ParsedLine *parsed,
    int line_number
)
{
    OpcodeInfo *opcode;

    add_code_label(context, parsed, line_number);

    opcode = find_opcode(parsed->name);
    if (opcode == 0) {
        mark_error(context, line_number, ERROR_UNKNOWN_OPCODE, parsed->name);
        context->ic += INSTRUCTION_SIZE;
        return;
    }

    if (is_r_arithmetic_name(parsed->name)) {
        process_r_arithmetic(context, parsed, opcode, line_number);
    } else if (is_r_copy_name(parsed->name)) {
        process_r_copy(context, parsed, opcode, line_number);
    } else if (is_i_immediate_name(parsed->name)) {
        process_i_immediate(context, parsed, opcode, line_number);
    } else if (is_i_branch_name(parsed->name)) {
        process_i_branch(context, parsed, opcode, line_number);
    } else if (strcmp(parsed->name, "jmp") == 0) {
        process_jmp(context, parsed, opcode, line_number);
    } else if (strcmp(parsed->name, "la") == 0 || strcmp(parsed->name, "call") == 0) {
        process_j_label(context, parsed, opcode, line_number);
    } else if (strcmp(parsed->name, "hlt") == 0) {
        process_hlt(context, parsed, opcode, line_number);
    }

    context->ic += INSTRUCTION_SIZE;
}

void init_first_pass_context(
    FirstPassContext *context
)
{
    if (context == 0) {
        return;
    }

    context->symbols = 0;
    init_data_image(&context->data_image);
    init_code_image(&context->code_image);
    context->ic = INITIAL_IC;
    context->dc = INITIAL_DC;
    context->icf = 0;
    context->dcf = 0;
    context->external_usages = 0;
    context->has_errors = 0;
}

void free_first_pass_context(
    FirstPassContext *context
)
{
    if (context == 0) {
        return;
    }

    free_symbol_table(context->symbols);
    context->symbols = 0;
    clear_data_image(&context->data_image);
    clear_code_image(&context->code_image);
    free_external_usages(context->external_usages);
    context->external_usages = 0;
}

static int run_first_pass_file(
    const char *base_name,
    FirstPassContext *context,
    int process_instructions,
    int update_data_addresses
)
{
    char filename[MAX_FILENAME_LENGTH];
    char line[MAX_LINE_LENGTH + 2];
    FILE *file;
    ParsedLine parsed;
    int line_number;

    if (context == 0) {
        return 0;
    }

    init_first_pass_context(context);

    if (base_name == 0 ||
        strlen(base_name) + strlen(AM_EXTENSION) >= MAX_FILENAME_LENGTH) {
        printf("Error: file name is too long %s\n", base_name == 0 ? "" : base_name);
        context->has_errors = 1;
        return 0;
    }

    build_filename(base_name, AM_EXTENSION, filename);
    file = fopen(filename, "r");
    if (file == 0) {
        printf("Error: cannot open file %s\n", filename);
        context->has_errors = 1;
        return 0;
    }

    line_number = 0;
    while (fgets(line, sizeof(line), file) != 0) {
        line_number++;

        if (!parse_line(line, &parsed, line_number)) {
            context->has_errors = 1;
            continue;
        }

        if (parsed.type == LINE_EMPTY || parsed.type == LINE_COMMENT) {
            continue;
        }

        if (parsed.type == LINE_INSTRUCTION) {
            if (process_instructions) {
                process_instruction(context, &parsed, line_number);
            }
            continue;
        }

        if (parsed.type == LINE_DIRECTIVE) {
            process_directive(context, &parsed, line_number);
        }
    }

    fclose(file);

    context->icf = context->ic;
    context->dcf = context->dc;

    if (!context->has_errors && update_data_addresses) {
        update_data_symbols(context->symbols, context->icf);
        update_data_image_addresses(&context->data_image, context->icf);
    }

    return !context->has_errors;
}

int run_first_pass_directives_only(
    const char *base_name,
    FirstPassContext *context
)
{
    return run_first_pass_file(base_name, context, 0, 0);
}

int run_first_pass(
    const char *base_name,
    FirstPassContext *context
)
{
    return run_first_pass_file(base_name, context, 1, 1);
}

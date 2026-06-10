#include <stdio.h>
#include <string.h>
#include "assembler.h"
#include "first_pass.h"

static int tests_failed = 0;

static void check(int condition, const char *message)
{
    if (!condition) {
        printf("FAIL: %s\n", message);
        tests_failed++;
    }
}

static void make_filename(
    const char *base_name,
    const char *extension,
    char *result
)
{
    strcpy(result, base_name);
    strcat(result, extension);
}

static int write_file(
    const char *filename,
    const char *content
)
{
    FILE *file;

    file = fopen(filename, "w");
    if (file == 0) {
        return 0;
    }

    fputs(content, file);
    fclose(file);
    return 1;
}

static void cleanup_case(
    const char *base_name
)
{
    char filename[MAX_FILENAME_LENGTH];

    make_filename(base_name, AM_EXTENSION, filename);
    remove(filename);
}

static int run_case(
    const char *base_name,
    const char *content,
    FirstPassContext *context
)
{
    char filename[MAX_FILENAME_LENGTH];

    cleanup_case(base_name);
    make_filename(base_name, AM_EXTENSION, filename);

    if (!write_file(filename, content)) {
        return 0;
    }

    return run_first_pass(base_name, context);
}

static int field_opcode(unsigned int word)
{
    return (int)((word >> 26) & 0x3f);
}

static int field_rs(unsigned int word)
{
    return (int)((word >> 21) & 0x1f);
}

static int field_rt(unsigned int word)
{
    return (int)((word >> 16) & 0x1f);
}

static int field_rd(unsigned int word)
{
    return (int)((word >> 11) & 0x1f);
}

static int field_funct(unsigned int word)
{
    return (int)((word >> 6) & 0x1f);
}

static int field_immed(unsigned int word)
{
    return (int)(word & 0xffff);
}

static int field_j_reg(unsigned int word)
{
    return (int)((word >> 25) & 1);
}

static int field_j_address(unsigned int word)
{
    return (int)(word & 0x1ffffff);
}

static void test_r_arithmetic(void)
{
    FirstPassContext context;
    Symbol *main_symbol;

    check(run_case(
        "fi_r_arithmetic",
        "MAIN: add $3,$5,$9\n"
        "sub $9,$8,$7\n",
        &context
    ), "r arithmetic pass succeeds");

    main_symbol = find_symbol(context.symbols, "MAIN");
    check(main_symbol != 0, "MAIN symbol exists");
    check(main_symbol != 0 && main_symbol->address == 100, "MAIN address");
    check(main_symbol != 0 && (main_symbol->attributes & SYMBOL_CODE) != 0,
        "MAIN code attribute");
    check(context.ic == 108, "r arithmetic ic");
    check(context.code_image.count == 2, "r arithmetic code count");
    check(context.code_image.items[0].unresolved_type == UNRESOLVED_NONE,
        "add unresolved none");
    check(context.code_image.items[1].unresolved_type == UNRESOLVED_NONE,
        "sub unresolved none");
    check(field_opcode(context.code_image.items[0].word) == 0, "add opcode");
    check(field_rs(context.code_image.items[0].word) == 3, "add rs");
    check(field_rt(context.code_image.items[0].word) == 5, "add rt");
    check(field_rd(context.code_image.items[0].word) == 9, "add rd");
    check(field_funct(context.code_image.items[0].word) == 1, "add funct");

    free_first_pass_context(&context);
    cleanup_case("fi_r_arithmetic");
}

static void test_r_copy(void)
{
    FirstPassContext context;

    check(run_case(
        "fi_r_copy",
        "move $23,$2\n"
        "mvhi $1,$31\n"
        "mvlo $4,$5\n",
        &context
    ), "r copy pass succeeds");

    check(context.ic == 112, "r copy ic");
    check(context.code_image.count == 3, "r copy count");
    check(field_rs(context.code_image.items[0].word) == 23, "move rs destination");
    check(field_rt(context.code_image.items[0].word) == 0, "move rt zero");
    check(field_rd(context.code_image.items[0].word) == 2, "move rd source");
    check(field_rs(context.code_image.items[1].word) == 1, "mvhi rs destination");
    check(field_rd(context.code_image.items[1].word) == 31, "mvhi rd source");
    check(field_rs(context.code_image.items[2].word) == 4, "mvlo rs destination");
    check(field_rd(context.code_image.items[2].word) == 5, "mvlo rd source");

    free_first_pass_context(&context);
    cleanup_case("fi_r_copy");
}

static void test_i_arithmetic(void)
{
    FirstPassContext context;

    check(run_case(
        "fi_i_arithmetic",
        "addi $9,-45,$8\n"
        "ori $1,+5,$2\n",
        &context
    ), "i arithmetic pass succeeds");

    check(context.ic == 108, "i arithmetic ic");
    check(context.code_image.count == 2, "i arithmetic count");
    check(field_rs(context.code_image.items[0].word) == 9, "addi rs");
    check(field_rt(context.code_image.items[0].word) == 8, "addi rt");
    check(field_immed(context.code_image.items[0].word) == 65491, "addi immediate");
    check(field_rs(context.code_image.items[1].word) == 1, "ori rs");
    check(field_rt(context.code_image.items[1].word) == 2, "ori rt");
    check(field_immed(context.code_image.items[1].word) == 5, "ori immediate");

    free_first_pass_context(&context);
    cleanup_case("fi_i_arithmetic");
}

static void test_i_memory(void)
{
    FirstPassContext context;

    check(run_case(
        "fi_i_memory",
        "lh $9,34,$2\n"
        "sw $7,-28,$18\n",
        &context
    ), "i memory pass succeeds");

    check(context.ic == 108, "i memory ic");
    check(context.code_image.count == 2, "i memory count");
    check(field_rs(context.code_image.items[0].word) == 9, "lh rs");
    check(field_rt(context.code_image.items[0].word) == 2, "lh rt");
    check(field_immed(context.code_image.items[0].word) == 34, "lh immediate");
    check(field_rs(context.code_image.items[1].word) == 7, "sw rs");
    check(field_rt(context.code_image.items[1].word) == 18, "sw rt");
    check(field_immed(context.code_image.items[1].word) == 65508, "sw immediate");

    free_first_pass_context(&context);
    cleanup_case("fi_i_memory");
}

static void test_i_branch_unresolved(void)
{
    FirstPassContext context;
    Symbol *loop_symbol;

    check(run_case(
        "fi_i_branch",
        "LOOP: bne $31,$9,LOOP\n"
        "blt $5,$24,END\n",
        &context
    ), "i branch pass succeeds");

    loop_symbol = find_symbol(context.symbols, "LOOP");
    check(loop_symbol != 0 && loop_symbol->address == 100, "LOOP address");
    check(context.code_image.count == 2, "branch count");
    check(context.code_image.items[0].unresolved_type == UNRESOLVED_BRANCH_LABEL,
        "bne unresolved branch");
    check(strcmp(context.code_image.items[0].unresolved_label, "LOOP") == 0,
        "bne unresolved label");
    check(field_immed(context.code_image.items[0].word) == 0, "bne immed zero");
    check(context.code_image.items[1].unresolved_type == UNRESOLVED_BRANCH_LABEL,
        "blt unresolved branch");
    check(strcmp(context.code_image.items[1].unresolved_label, "END") == 0,
        "blt unresolved label");
    check(field_immed(context.code_image.items[1].word) == 0, "blt immed zero");

    free_first_pass_context(&context);
    cleanup_case("fi_i_branch");
}

static void test_j_register(void)
{
    FirstPassContext context;

    check(run_case("fi_j_register", "jmp $4\n", &context), "j register pass succeeds");

    check(field_j_reg(context.code_image.items[0].word) == 1, "jmp register bit");
    check(field_j_address(context.code_image.items[0].word) == 4, "jmp register address");
    check(context.code_image.items[0].unresolved_type == UNRESOLVED_NONE,
        "jmp register unresolved none");

    free_first_pass_context(&context);
    cleanup_case("fi_j_register");
}

static void test_j_label_unresolved(void)
{
    FirstPassContext context;

    check(run_case(
        "fi_j_label",
        "jmp NEXT\n"
        "la K\n"
        "call val1\n",
        &context
    ), "j label pass succeeds");

    check(context.code_image.count == 3, "j label count");
    check(context.code_image.items[0].unresolved_type == UNRESOLVED_J_LABEL,
        "jmp label unresolved");
    check(strcmp(context.code_image.items[0].unresolved_label, "NEXT") == 0,
        "jmp label name");
    check(field_j_address(context.code_image.items[0].word) == 0, "jmp label address zero");
    check(context.code_image.items[1].unresolved_type == UNRESOLVED_J_LABEL,
        "la unresolved");
    check(strcmp(context.code_image.items[1].unresolved_label, "K") == 0, "la label");
    check(context.code_image.items[2].unresolved_type == UNRESOLVED_J_LABEL,
        "call unresolved");
    check(strcmp(context.code_image.items[2].unresolved_label, "val1") == 0,
        "call label");

    free_first_pass_context(&context);
    cleanup_case("fi_j_label");
}

static void test_hlt(void)
{
    FirstPassContext context;
    Symbol *end_symbol;

    check(run_case("fi_hlt", "END: hlt\n", &context), "hlt pass succeeds");

    end_symbol = find_symbol(context.symbols, "END");
    check(end_symbol != 0 && end_symbol->address == 100, "END address");
    check(field_opcode(context.code_image.items[0].word) == 63, "hlt opcode");
    check(context.code_image.items[0].unresolved_type == UNRESOLVED_NONE,
        "hlt unresolved none");
    check(context.code_image.count == 1, "hlt code count");

    free_first_pass_context(&context);
    cleanup_case("fi_hlt");
}

static void test_instruction_errors(void)
{
    FirstPassContext context;
    Symbol *after_unknown;

    check(!run_case(
        "fi_errors",
        "unknown $1,$2\n"
        "AFTERUNKNOWN: add $1,$2,$3\n"
        "add $1,$2\n"
        "add $1,$2,$3,$4\n"
        "add $1,$2,$33\n"
        "addi $1,999999,$2\n"
        "jmp\n"
        "la $1\n"
        "hlt $1\n",
        &context
    ), "instruction errors fail");
    check(context.has_errors, "instruction errors has_errors");
    after_unknown = find_symbol(context.symbols, "AFTERUNKNOWN");
    check(after_unknown != 0 && after_unknown->address == 104,
        "unknown opcode still advances IC");

    free_first_pass_context(&context);
    cleanup_case("fi_errors");
}

static void test_data_update_after_code(void)
{
    FirstPassContext context;
    Symbol *main_symbol;
    Symbol *str_symbol;
    Symbol *k_symbol;

    check(run_case(
        "fi_data_update",
        "MAIN: add $3,$5,$9\n"
        "hlt\n"
        "STR: .asciz \"a\"\n"
        "K: .dw 31\n",
        &context
    ), "data update pass succeeds");

    main_symbol = find_symbol(context.symbols, "MAIN");
    str_symbol = find_symbol(context.symbols, "STR");
    k_symbol = find_symbol(context.symbols, "K");

    check(context.icf == 108, "data update icf");
    check(context.dcf == 6, "data update dcf");
    check(main_symbol != 0 && main_symbol->address == 100, "MAIN code address");
    check(str_symbol != 0 && str_symbol->address == 108, "STR updated address");
    check(k_symbol != 0 && k_symbol->address == 110, "K updated address");
    check(context.data_image.items[0].address == 108, "data image first address");

    free_first_pass_context(&context);
    cleanup_case("fi_data_update");
}

int main(void)
{
    test_r_arithmetic();
    test_r_copy();
    test_i_arithmetic();
    test_i_memory();
    test_i_branch_unresolved();
    test_j_register();
    test_j_label_unresolved();
    test_hlt();
    test_instruction_errors();
    test_data_update_after_code();

    if (tests_failed != 0) {
        printf("%d first pass instruction tests failed\n", tests_failed);
        return 1;
    }

    printf("All first pass instruction tests passed\n");
    return 0;
}

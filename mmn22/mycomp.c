#include <stdio.h>

#include "parser.h"
#include "complex.h"

#define MAX_LEN 256

/*
 Six complex variables required by the assignment.
 All variables are initialized to zero at program startup.
 */
complex A = {0, 0};
complex B = {0, 0};
complex C = {0, 0};
complex D = {0, 0};
complex E = {0, 0};
complex F = {0, 0};

/*
 Maps variable names (A-F) to the actual complex variables.
 Used by the parser when resolving command parameters.
 */
complex_store compNums[] = {
    {'A', &A},
    {'B', &B},
    {'C', &C},
    {'D', &D},
    {'E', &E},
    {'F', &F},
    {'#', NULL}
};

/*
 Maps command names to internal command types.
 Used when parsing the command name.
 */
command_info cmd[] = {
    {"read_comp", CMD_READ_COMP},
    {"print_comp", CMD_PRINT_COMP},
    {"add_comp", CMD_ADD_COMP},
    {"sub_comp", CMD_SUB_COMP},
    {"mult_comp_real", CMD_MULT_COMP_REAL},
    {"mult_comp_img", CMD_MULT_COMP_IMG},
    {"mult_comp_comp", CMD_MULT_COMP_COMP},
    {"abs_comp", CMD_ABS_COMP},
    {"stop", CMD_STOP},
    {"not_valid", CMD_INVALID}
};

/* Execute a parsed command. */
static void run_command(parms *p);

static void run_command(parms *p)
{
    switch (p->type)
    {
        case CMD_READ_COMP:

            /* Assign values to a complex variable */
            read_comp(p->comp1, p->real, p->imag);

            break;

        case CMD_PRINT_COMP:

            print_comp(p->comp1);

            break;

        case CMD_ADD_COMP:

            add_comp(p->comp1, p->comp2);

            break;

        case CMD_SUB_COMP:

            sub_comp(p->comp1, p->comp2);

            break;

        case CMD_MULT_COMP_REAL:

            mult_comp_real(p->comp1, p->real);

            break;

        case CMD_MULT_COMP_IMG:

            mult_comp_img(p->comp1, p->imag);

            break;

        case CMD_MULT_COMP_COMP:

            mult_comp_comp(p->comp1, p->comp2);

            break;

        case CMD_ABS_COMP:

            abs_comp(p->comp1);

            break;

        default:

            break;
    }
}

int main(void)
{
    char line[MAX_LEN];

    parms p;

    parse_status status;

    int running = 1;
    int stop_received = 0;

    while (running)
    {
        /* Reset parsed command structure */
        clear_parms(&p);

        printf("Please enter a command:\n");

        /*
         EOF is handled separately from the stop command.
         The assignment requires detecting termination
         without a valid stop command.
         */
        if (fgets(line, sizeof(line), stdin) == NULL)
        {
            running = 0;
            continue;
        }

        /* Echo the received input line */
        printf("%s", line);

        /* Parse the input line */
        status = parse_line(line, &p);

        if (status != PARSE_OK)
        {
            print_error(status);
            continue;
        }

        /* Normal program termination */
        if (p.type == CMD_STOP)
        {
            stop_received = 1;
            running = 0;
        }
        else
        {
            run_command(&p);
        }
    }

    /*
     According to the assignment,
     reaching EOF without stop is an error.
     */
    if (!stop_received)
    {
        printf("Error: missing stop command\n");
    }

    return 0;
}
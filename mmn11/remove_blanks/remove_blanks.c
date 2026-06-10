#include <stdio.h>
#include <ctype.h>

#define MAX_LENGTH 256  /* Maximum length of input string */

/*
 * Function: remove_blanks
 * -----------------------
 * Removes all whitespace characters (spaces, tabs, newlines)
 * from the given string.
 *
 * The function scans the string character by character.
 * Each non-whitespace character is copied to the front of the string.
 * The result is a compact string without blanks.
 *
 * Parameters:
 *   line - input string (modified in-place)
 *
 * Return value:
 *   None
 */
void remove_blanks(char line[]) {
    int read_index = 0;     /* index for scanning original string */
    int write_index = 0;    /* index for writing cleaned string */

    /* Print original string */
    printf("The string as received by the function:\n");
    printf("\"%s\"\n", line);

    /* Iterate over the string */
    while (line[read_index] != '\0') {
         /* If current character is not whitespace, keep it */
        if (! isspace(line[read_index])) {
            line[write_index] = line[read_index];
            write_index++;
        }
        read_index++;
    }
    /* Add string terminator */
    line[write_index] = '\0';

    /* Print processed string */
    printf("The string at the end of the function:\n");
    printf("\"%s\"\n", line);
}

/*
 * Function: main
 * --------------
 * Reads a string from standard input and removes all whitespace
 * characters using the remove_blanks function.
 *
 * The program demonstrates string processing in C.
 *
 * Return value:
 *   0 on success
 */
int main()
{
    char line [MAX_LENGTH] = {'\0'};    /* input buffer */

    printf("Please enter a string:\n");

    /* Read input from user */
    fgets(line, MAX_LENGTH, stdin);

    /* Print original input */
    printf("The Input string:\n\"%s\"\n", line);

    /* Remove blanks from the string */
    remove_blanks(line);

    return 0;
}
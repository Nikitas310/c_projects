#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

/* Checks whether the file contains at least n bytes */
int isFileBigEnough(int, char *);

/* Checks whether the file can be opened */
int isFileExists(const char *);

/* Prints the decimal ASCII code of the nth character in a file */
void printTheNChar(int, char *);


int main(int argc, char **argv)
{
    int n, i;

    /* At least two arguments are required: n and one file name */
    if (argc < 3) {
        printf("Argument error: You have to pass at least 2 arguments. Program stopped.\n");
        return 0;
    }

    /* Convert the first argument to an integer */
    n = atoi(argv[1]);

    /* n must be a positive integer */
    if (n <= 0) {
        printf("Argument error: First argument has to be a positive integer. Program stopped.\n");
        return 0;
    }

    /* Process each file passed in the command line */
    for (i = 2; i < argc; i++) {

        /* Skip the file if it cannot be opened */
        if (!(isFileExists(argv[i]))) {
            printf("File %s can't be opened.\n", argv[i]);
            continue;
        }

        /* Skip the file if it is shorter than n bytes */
        if (!(isFileBigEnough(n, argv[i]))) {
            printf("File %s isn't big enough.\n", argv[i]);
            continue;
        }

        /* Print the nth character */
        printTheNChar(n, argv[i]);
    }

    return 0;
}

/* Opens the file, moves to the nth character, and prints its ASCII code in decimal */
void printTheNChar(int n, char *path)
{
    FILE *fp = fopen(path, "r");

    /* Move to byte number n-1 (because offsets start from 0) */
    fseek(fp, n - 1, SEEK_SET);

    printf("The %d char in file %s is %i\n", n, path, fgetc(fp));

    fclose(fp);
}

/* Returns 1 if the file can be opened, otherwise returns 0 */
int isFileExists(const char *path)
{
    FILE *fptr = fopen(path, "r");

    if (fptr == NULL)
        return 0;

    fclose(fptr);

    return 1;
}

/* Returns 1 if the file contains at least n bytes, otherwise returns 0 */
int isFileBigEnough(int n, char *path)
{
    struct stat stats;

    stat(path, &stats);

    if (n > stats.st_size)
        return 0;

    return 1;
}
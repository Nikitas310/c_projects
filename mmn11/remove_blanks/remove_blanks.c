#include <stdio.h>
#include <ctype.h>

#define MAX_LENGTH 64

void remove_blanks(char line[MAX_LENGTH]) {
    int i = 0;
    int last_checked = 0;

    printf("The string as received by the function:\n");
    printf("%s\n", line);

    while (line[i] != '\0') {
        if (! isspace(line[i])) {
            line[last_checked++] = line[i];
        }
        i++;
    }
    line[last_checked] = '\0';

    printf("\n");
    printf("The string at the end of the function:\n");
    printf("%s\n", line);

    printf("\n");
}

int main()
{
    char line [MAX_LENGTH];
    char c;
    int i;

    for (i = 0; i < MAX_LENGTH; ++i){
        line[i] = '\0';
    }

    printf("The Input string:\n");

    i = 0;
    while ((c = getchar()) != '\n' && c != EOF && i < MAX_LENGTH - 1) {
        line[i++] = c;
    }

    remove_blanks(line);

    return 0;
}
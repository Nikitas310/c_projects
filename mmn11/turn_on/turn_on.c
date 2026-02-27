#include <stdio.h>

long turn_on(long num);

int main() {

    long num;
    long new_num;
    int bits = sizeof(num) * 8;
    int i = 0;

    printf("Enter a number:\n");
    scanf("%ld", &num);

    printf("The original number:\n");
    for (i = bits - 1; i >= 0; i--)
        printf("%ld", (num >> i) & 1);
    printf("\n");
    printf("In decimal base: %ld\n", num);

    new_num = turn_on(num);

    if (num != new_num){
        printf("After turning on the 17th bit from the right: a change has been made\n");
        for (i = bits - 1; i >= 0; i--)
            printf("%ld", (new_num >> i) & 1);
        printf("\n");
        printf("In decimal base: %ld\n", new_num);

    }
    else
        printf("The 17th bit from the right is on, so there is no change\n");

    return 0;
}

long turn_on(long num) {
    long mask = 1L << 16;
    return num | mask;
}
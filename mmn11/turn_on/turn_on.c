#include <stdio.h>

/*
 * Function: turn_on
 * -----------------
 * Turns on (sets to 1) the 17th bit from the right in the given number.
 *
 * The function creates a mask where only the 17th bit is set to 1
 * (bit index 16, since counting starts from 0).
 * Then it uses bitwise OR to ensure this bit is turned on.
 *
 * Parameters:
 *   num - input number
 *
 * Return value:
 *   The number after setting the 17th bit
 */
long turn_on(long);

/*
 * Function: print_binary
 * ----------------------
 * Prints the binary representation of a given number.
 *
 * The function iterates over all bits of the number from the most
 * significant bit to the least significant bit, and prints each bit.
 *
 * Parameters:
 *   num - number to print in binary format
 *
 * Return value:
 *   None
 */
void print_binary(long);

/*
 * Function: print_with_commas
 * ---------------------------
 * Prints a given integer number with commas as thousands separators.
 *
 * This implementation uses only arithmetic operations (no string conversion).
 *
 * Algorithm:
 * 1. If the number is negative, print '-' and work with its absolute value.
 * 2. Find the largest power of 1000 that is less than or equal to the number.
 * 3. Print the most significant group (without leading zeros).
 * 4. For each remaining group:
 *    - Divide by the current divisor to extract the next group (0–999).
 *    - Print it using %03ld to ensure leading zeros.
 *    - Reduce the number using modulo.
 *    - Decrease the divisor by a factor of 1000.
 *
 * Parameters:
 *   num - input number (long)
 *
 * Return value:
 *   None
 */
void print_with_commas(long);

/*
 * Function: main
 * --------------
 * Reads a number from the user, prints its binary and decimal
 * representations, then turns on the 17th bit and prints the result.
 *
 * The program demonstrates bitwise operations and binary representation.
 *
 * Return value:
 *   0 on success
 */
int main() {

    long num;       /* original number */
    long new_num;   /* number after modification */

    /* Read input from user */
    printf("Enter a number:\n");
    scanf("%ld", &num);

    /* Print original number */
    printf("The original number:\n");
    print_binary(num);
    printf("In decimal base:\n");
    print_with_commas(num);

    /* Turn on the 17th bit */
    new_num = turn_on(num);

    /* Check if a change occurred */
    if (num != new_num){
        printf("After turning on the 17th bit from the right: a change has been made\n");
        print_binary(new_num);
        printf("In decimal base:\n");
        print_with_commas(new_num);
    }
    else
        printf("The 17th bit from the right is on, so there is no change\n");

    return 0;
}

/*
 * Implementation of turn_on:
 * Creates a mask with the 17th bit set and applies OR operation.
 */
long turn_on(long num) {
    long mask = 1L << 16;   /* mask with only the 17th bit set */
    return num | mask;
}

/*
 * Implementation of print_binary:
 * Prints each bit of the number from left (MSB) to right (LSB).
 */
void print_binary(long num) {
    int bits = sizeof(num) * 8;     /* total number of bits */
    int i = 0;
    for (i = bits - 1; i >= 0; i--)
        printf("%ld", (num >> i) & 1);
    printf("\n");
}

#include <stdio.h>

/*
 * Implementation of print_with_commas:
 * Prints a given integer number with commas as thousands separators.
 */
void print_with_commas(long num) {
    long divisor = 1;

    /* Handle negative numbers */
    if (num < 0) {
        printf("-");
        num = -num;
    }

    /* Find largest power of 1000 */
    while (num / divisor >= 1000) {
        divisor *= 1000;
    }

    /* Print first group (no leading zeros) */
    printf("%ld", num / divisor);
    num %= divisor;

    /* Print remaining groups with leading zeros */
    while (divisor > 1) {
        divisor /= 1000;
        printf(",%03ld", num / divisor);
        num %= divisor;
    }

    printf("\n");
}
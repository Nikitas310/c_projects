#include <stdio.h>
#include <math.h>
#include "complex.h"

/* Assign real and imaginary parts to a complex number. */
void read_comp(complex *num, double real, double imag)
{
    num->real = real;
    num->imag = imag;
}

/* Print a complex number. */
void print_comp(const complex *num)
{
    printf("%.2f + (%.2f)i\n", num->real, num->imag);
}

/* Add two complex numbers and print the result. */
void add_comp(const complex *num1, const complex *num2)
{
    double sum_real;
    double sum_imag;

    sum_real = num1->real + num2->real;
    sum_imag = num1->imag + num2->imag;

    printf("%.2f + (%.2f)i\n", sum_real, sum_imag);
}

/* Subtract two complex numbers and print the result. */
void sub_comp(const complex *num1, const complex *num2)
{
    double sub_real;
    double sub_imag;

    sub_real = num1->real - num2->real;
    sub_imag = num1->imag - num2->imag;

    printf("%.2f + (%.2f)i\n", sub_real, sub_imag);
}

/* Multiply a complex number by a real number and print the result. */
void mult_comp_real(const complex *num1, double num2)
{
    double mult_real;
    double mult_imag;

    mult_real = num2 * num1->real;
    mult_imag = num2 * num1->imag;

    printf("%.2f + (%.2f)i\n", mult_real, mult_imag);
}

/* Multiply a complex number by a pure imaginary number */
void mult_comp_img(const complex *num1, double num2)
{
    double mult_real;
    double mult_imag;

    mult_real = (-num2) * num1->imag;
    mult_imag = num2 * num1->real;

    printf("%.2f + (%.2f)i\n", mult_real, mult_imag);
}

/* Multiply two complex numbers and print the result. */
void mult_comp_comp(const complex *num1, const complex *num2)
{
    double mult_real;
    double mult_imag;

    mult_real = num1->real * num2->real - num1->imag * num2->imag;

    mult_imag = num1->real * num2->imag + num1->imag * num2->real;

    printf("%.2f + (%.2f)i\n", mult_real, mult_imag);
}

/* Calculate and print the absolute value */
void abs_comp(const complex *num1)
{
    double result;

    result = sqrt(num1->real * num1->real + num1->imag * num1->imag);

    printf("%.2f\n", result);
}
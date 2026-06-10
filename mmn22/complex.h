#ifndef COMPLEX_H
#define COMPLEX_H

/* Complex number structure. */
typedef struct {
    double real;
    double imag;
} complex;

/* Complex number operations. */
void read_comp(complex *num, double real, double imag);
void print_comp(const complex *num);
void add_comp(const complex *num1, const complex *num2);
void sub_comp(const complex *num1, const complex *num2);
void mult_comp_real(const complex *num1, double num2);
void mult_comp_img(const complex *num1, double num2);
void mult_comp_comp(const complex *num1, const complex *num2);
void abs_comp(const complex *num1);

#endif
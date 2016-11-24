#ifndef __MATRIX_H__
#define __MATRIX_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MATRIX_OK 0
#define MATRIX_ERR 1

typedef struct {
    long long int rows;
    long long int cols;
    double *values;
} Matrix;

void Matrix_Multiply(Matrix a, Matrix b, Matrix c);

void Matrix_Add(Matrix a, Matrix b, Matrix c);

void Matrix_Scale(Matrix a, double n);

void Matrix_Print(Matrix m);

void Matrix_Test(void);

#endif  /* __MATRIX_H__ */



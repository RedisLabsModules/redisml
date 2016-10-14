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

void MatrixMultiply(Matrix a, Matrix b, Matrix c);
void MatrixAdd(Matrix a, Matrix b, Matrix c);
void MatrixScale(Matrix a, double n, Matrix c);
void MatrixPrint(Matrix m);
void MatrixTest(void);
#endif  // __MATRIX_H__

//

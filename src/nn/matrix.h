#ifndef __NN_MATRIX_H__
#define __NN_MATRIX_H__

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    size_t rows;
    size_t cols;
    float *values;
} Matrix;

Matrix *Matrix_New(size_t rows, size_t cols);
void Matrix_Free(Matrix *m);
void Matrix_Print(Matrix *m);
int Matrix_IsEqual(Matrix *a, Matrix *b);
void Matrix_Transpose(Matrix *m, Matrix *mt);
void Matrix_Multiply(Matrix *a, Matrix *b, Matrix* c); 
void Matrix_Add(Matrix *a, Matrix *b, Matrix* c); 

#define MATRIX(m,i,j) m->values[(i) * m->cols + j]

#endif //__NN_MATRIX_H__

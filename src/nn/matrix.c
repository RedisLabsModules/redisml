#ifdef USE_BLAS
#include <cblas.h>
#endif
#include "matrix.h"


Matrix *Matrix_New(size_t rows, size_t cols) {
    Matrix *m = malloc(sizeof(Matrix));
    m->rows = rows;
    m->cols = cols;
    m->values = calloc(rows * cols, sizeof(float));
    return m;
}

void Matrix_Free(Matrix *m) {
    free(m->values);
    free(m);
}

void Matrix_Print(Matrix *m) {
    printf("\n");
    for (int i = 0; i < m->rows; i++) {
        for (int j = 0; j < m->cols; j++){
            printf("%.2f, ", MATRIX(m, i, j));
        }
        printf("\n");
    }
}

void Matrix_Transpose(Matrix *m, Matrix *mt) {
    for (int i = 0; i < m->rows; i++) {
        for (int j = 0; j < m->cols; j++){
            MATRIX(mt, j, i) = MATRIX(m, i, j);
        }
    }
}

int Matrix_IsEqual(Matrix *a, Matrix *b) {
    if (a->cols != b->cols || a->rows != b->rows) {
       return 0;
    }

    for (int i = 0; i < a->rows * a->cols; i++) {
        if (a->values[i] != b->values[i]) {
            return 0;
        }
    }
    return 1;
}

void Matrix_Multiply(Matrix *a, Matrix *b, Matrix* c) {
#ifdef USE_BLAS
    cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, a->rows, b->cols, a->cols,
        1, a->values, a->cols, b->values, b->cols, 0, c->values, b->cols);
#else
    Matrix *bt = Matrix_New(b->cols, b->rows);
    Matrix_Transpose(b, bt);
    for (int ai = 0; ai < a->rows; ai++) {
       for (int bti = 0; bti < a->rows; bti++) {
          for (int j = 0; j < a->cols; j++) {
              MATRIX(c, ai, bti) += MATRIX(a, ai, j) * MATRIX(bt, bti, j);
          }
       }
    }
    Matrix_Free(bt);
#endif
}

void Matrix_Add(Matrix *a, Matrix *b, Matrix *c) {
    for (int i = 0; i < a->rows * a->cols; i++) {
        c->values[i] = a->values[i] + b->values[i];
    }
}

 

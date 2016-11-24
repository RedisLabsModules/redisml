#include <stdio.h>
#include <stdlib.h>
#include <cblas.h>
#include "matrix.h"

void Matrix_Multiply(Matrix a, Matrix b, Matrix c) {
    /* for dimentions:
     * a[m*k],b[k*n],c[m*n]
     * We use:
     * RowMajor, NoTrans, NoTrans, m, n, k, 1, A, k, B, n, 0, C, n);
     */

    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, a.rows, b.cols, a.cols,
                1, a.values, a.cols, b.values, b.cols, 0, c.values, b.cols);
}

void Matrix_Print(Matrix m) {
    printf("=====\n");
    printf("rows: %lld, columns: %lld\n", m.rows, m.cols);
    for (int i = 0; i < m.rows; i++) {
        for (int j = 0; j < m.cols; j++) {
            printf("%lf ", m.values[i * m.cols + j]);
        }
        printf("\n");
    }
}

void Matrix_Add(Matrix a, Matrix b, Matrix c) {
    for (int i = 0; i < a.rows * a.cols; i++) {
        c.values[i] = a.values[i] + b.values[i];
    }
}

void Matrix_Scale(Matrix a, double n) {
    cblas_dscal(a.rows * a.cols, n, a.values, 1);
}

void Matrix_Test(void) {
    double *A, *B, *C;
    int m, n, k, i;
    double alpha, beta;
    m = 2000, k = 200, n = 1000;
    printf(
            " Initializing data for matrix multiplication C=A*B for matrix \n"
                    " A(%ix%i) and matrix B(%ix%i)\n",
            m, k, k, n);
    alpha = 1.0;
    beta = 0.0;

    A = (double *) malloc(m * k * sizeof(double));
    B = (double *) malloc(k * n * sizeof(double));
    C = (double *) malloc(m * n * sizeof(double));
    if (A == NULL || B == NULL || C == NULL) {
        printf("\n ERROR: Can't allocate memory for matrices. Aborting... \n\n");
        free(A);
        free(B);
        free(C);
        return;
    }

    printf(" Intializing matrix data \n");
    for (i = 0; i < (m * k); i++) {
        A[i] = (double) (i + 1);
    }

    for (i = 0; i < (k * n); i++) {
        B[i] = (double) (-i - 1);
    }

    for (i = 0; i < (m * n); i++) {
        C[i] = 0.0;
    }

    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, m, n, k, alpha, A, k,
                B, n, beta, C, n);
    printf("\n Computations completed.\n");
    free(A);
    free(B);
    free(C);
}
#include "test_util.h"
#include "../matrix.h"

int testMatrixTranspose() {
    Matrix *m = Matrix_New(3, 4);

    for ( int i=0; i < m->rows * m->cols; i++) {
        m->values[i] = i + 1;
    }
    
    Matrix *mt = Matrix_New(4, 3);
    Matrix_Transpose(m, mt);
    Matrix *mtt = Matrix_New(3, 4);
    Matrix_Transpose(mt, mtt);
    ASSERT_EQUAL(1, Matrix_IsEqual(m,mtt));
    Matrix_Free(m);
    Matrix_Free(mt);
    Matrix_Free(mtt);
    return 0;
}

int testMatrixMultiply() {
    Matrix *a = Matrix_New(2, 3);

    for ( int i=0; i < a->rows * a->cols; i++) {
        a->values[i] = i + 1;
    }
    Matrix_Print(a);
    
    Matrix *b = Matrix_New(3, 2);

    for ( int i=0; i < b->rows * b->cols; i++) {
        b->values[i] = i + 7;
    }
    Matrix_Print(b);

    Matrix *c = Matrix_New(2, 2);
    Matrix_Multiply(a, b, c);
    Matrix_Print(c);
    Matrix *test = Matrix_New(2, 2);
    test->values[0] = 58.0f;
    test->values[1] = 64.0f;
    test->values[2] = 139.0f;
    test->values[3] = 154.0f;
    ASSERT_EQUAL(1, Matrix_IsEqual(c, test));
    Matrix_Free(a);
    Matrix_Free(b);
    Matrix_Free(c);
    Matrix_Free(test);
    return 0;
}


TEST_MAIN({
    TESTFUNC(testMatrixTranspose);
    TESTFUNC(testMatrixMultiply);
})


#ifndef __TF_H__
#define __TF_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tensorflow/c/c_api.h"

#define REG_OK 0
#define REG_ERR 1

typedef struct {
    double intercept;
    double *coefficients;
    int clen;
} Tensor;

double TFPredict(const double **features, Tensor *ltr);

//double LogRegPredict(double *features, LinReg *);

#endif  // __TF_H__


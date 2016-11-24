#ifndef __REG_H__
#define __REG_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define REG_OK 0
#define REG_ERR 1

typedef struct {
    double intercept;
    double *coefficients;
    int clen;
} LinReg;

double LinRegPredict(double *features, LinReg *);

double LogRegPredict(double *features, LinReg *);

#endif  // __REG_H__

//

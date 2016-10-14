#include "reg.h"

double LinRegPredict(double *features, LinReg *lr) {
  double p = 0;
  for (int i = 0; i < lr->clen; i++) {
    p += features[i] * lr->coefficients[i];
  }
  return p + lr->intercept;
}
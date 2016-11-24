#include "reg.h"
#include "math.h"

double LinRegPredict(double *features, LinReg *lr) {
    double p = 0;
    for (int i = 0; i < lr->clen; i++) {
        p += features[i] * lr->coefficients[i];
    }
    return p + lr->intercept;
}

// private
// val score : Vector = > Double = (features) = > {
//   val m = margin(features) 1.0 / (1.0 + math.exp(-m))
// }

double LogRegPredict(double *features, LinReg *lr) {
    double m = LinRegPredict(features, lr);
    double p = 1.0 / (1.0 + exp(-m));
    return p;
}
#ifndef __FEATUREVEC_H__
#define __FEATUREVEC_H__

#include <stdio.h>
#include <stdlib.h>

#define REDIS_ML_FV_ERROR_BAD_FORMAT "Feature vector parsing failed. Format should be : \'key:val,key:val\'..."

#define MAX_NUM_FIELDS 2048

typedef struct {
    char *key;
    double value;
} Feature;

typedef struct {
    double Result;
    Feature *Features;
    size_t len;
    double NumericFields[MAX_NUM_FIELDS];
} FeatureVec;

int FeatureVec_Create(char *data, FeatureVec *fv);

void FeatureVec_Print(FeatureVec *ir);

double FeatureVec_GetValue(FeatureVec *ir, char *key);
double FeatureVec_NumericGetValue(FeatureVec *ir, int nkey);
#endif  /* __FEATUREVEC_H__ */

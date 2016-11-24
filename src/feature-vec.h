#ifndef __READER_H__
#define __READER_H__

#include <stdio.h>
#include <stdlib.h>

#define REDIS_ML_FV_ERROR_BAD_FORMAT "Feature vector parsing failed. Format should be : \'key:val,key:val\'..."

#define MAX_NUM_FIELDS 1024

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

int FeatureVec_Create(char *data, FeatureVec *ir);

void FeatureVec_Print(FeatureVec *ir);

double FeatureVec_GetValue(FeatureVec *ir, char *key);

#endif  /* __READER_H__ */

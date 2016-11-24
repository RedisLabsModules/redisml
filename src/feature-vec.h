#ifndef __READER_H__
#define __READER_H__

#include <stdio.h>
#include <stdlib.h>

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

void ReadInput(char *file, FeatureVec **rows);

int MakeFeatureVec(char *data, FeatureVec *ir);

void PrintFeatureVec(FeatureVec *ir);

double GetValue(FeatureVec *ir, char *key);

#endif  /* __READER_H__ */

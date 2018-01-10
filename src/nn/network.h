#ifndef __NETWORK_H__
#define __NETWORK_H__
#include "layer.h"

typedef enum {
    LINEAR,
    SSE,
    CES
} CostType;

float CostLinear (Matrix, Matrix);
float CostSSE (Matrix, Matrix);
float CostCES (Matrix, Matrix);

typedef struct {
    size_t nSamples;
    size_t featureSize;
    float *features;
    unsigned int *labels;
} DataSet; 

typedef struct {
    int nlayers;
    Layer **layers;
    DataSet *trainingData;
    DataSet *testData;
} Network;

void NN_SGD(Network *n, size_t cycles, size_t batchSize, float rate); 

void runMiniBatch(Network *n, int offset,size_t batchSize, float rate);

void backProp(Network *n, float *features, unsigned int label); 

void feedForward(Network *n, float *features);

#endif //__NETWORK_H__

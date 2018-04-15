#ifndef __NETWORK_H__
#define __NETWORK_H__
#include "layer.h"

typedef enum {
    LINEAR,
    SSE,
    CES
} CostType;

float CostLinear (float, float);
float CostSSE (float, float);
float CostCES (float, float);

float CostLinearDeriv (float, float);
float CostSSEDeriv (float, float);
float CostCESDeriv (float, float);

typedef struct {
    size_t nSamples;
    size_t featureSize;
    float *features;
    unsigned char *labels;
} DataSet; 

typedef struct {
    int nlayers;
    Layer **layers;
    float (* costFunc)(float, float);
    float (* costDerivativeFunc)(float, float);
    DataSet *trainingData;
    DataSet *testData;
} Network;

void NN_SGD(Network *n, size_t cycles, size_t batchSize, float rate); 

void runMiniBatch(Network *n, int offset,size_t batchSize, float rate);

void feedForward(Network *n, float *features);

#endif //__NETWORK_H__

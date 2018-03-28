#ifndef __LAYER_H__
#define __LAYER_H__
#include "matrix.h"
#include <math.h>

float Sigmoid(float x);
float SigmoidPrime(float x);

typedef enum {
    FULLY_CONNECTED,
    CONVOLUTIONAL,
    OUTPUT
} LayerType;

typedef enum {
    SIGMOID,
    SOFTMAX,
    RELU
} ActivationType;

typedef struct layer{
    LayerType type;
    ActivationType activationType;
    float (* activationFunc)(float x);
    float (* activationDerivativeFunc)(float x);
    size_t size;
    size_t inputSize;
    Matrix *weights;
    Matrix *biases;
    Matrix *weightsGrad;
    Matrix *biasesGrad;
    Matrix *activations;
} Layer;

Layer *Layer_Init(size_t size, size_t inputSize, LayerType type, ActivationType activationType);
void Layer_CalcActivations(Layer *l, Matrix *input);



#endif //__LAYER_H__

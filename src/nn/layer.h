#ifndef __LAYER_H__
#define __LAYER_H__
#include "matrix.h"
#include <math.h>

float Sigmoid(float x);
float SigmoidDeriv(float x);
float Relu(float x);
float ReluDeriv(float x);

typedef enum {
    FULLY_CONNECTED,
    CONVOLUTIONAL,
    MAX_POOLING,
    DROPOUT,
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
    Matrix *w;
    Matrix *b;
    Matrix *z;
    Matrix *wGrad;
    Matrix *bGrad;
    Matrix *a;
    Matrix *wDelta;
    Matrix *bDelta;
    Matrix *delta;
    Matrix **conv_a;
    Matrix **filters;
    size_t nfilters;
    size_t padding;
    size_t stride;
} Layer;

Layer *Layer_Init(size_t size, size_t inputSize, LayerType type, ActivationType activationType);
void Layer_CalcActivations(Layer *l, Matrix *input);
void Layer_ConvCalcActivations(Layer *l, Matrix *input);

void Softmax(Layer *l);


#endif //__LAYER_H__

#include "layer.h"

float Sigmoid(float x){
        return 1 / (1 + expf(-1 * x));
}

float SigmoidPrime(float x){
    return Sigmoid(x) * (1 - Sigmoid(x));
}

Layer *Layer_Init(size_t size, size_t inputSize, LayerType type, ActivationType activationType) {
    Layer *l = malloc(sizeof(Layer));
    l->type = type;
    l->activationType = activationType;
    l->size = size;
    l->inputSize = inputSize;
    l->weights = Matrix_New(size, inputSize);
    l->biases = Matrix_New(size, 1);
    l->weightsCostGrad = Matrix_New(size, inputSize);
    l->biasesCostGrad = Matrix_New(size, 1);
    l->activations = Matrix_New(size, 1);
    switch (activationType) {
        case SIGMOID:
            l->activationFunc = &Sigmoid;
            l->activationDerivativeFunc = &SigmoidPrime;
        default:
            l->activationFunc = &Sigmoid;
            l->activationDerivativeFunc = &SigmoidPrime;
    }
    return l;
}

void Layer_CalcActivations(Layer *l, Matrix *input) {
    Matrix_Multiply(input, l->weights, l->activations);
    Matrix_Add(l->activations, l->biases, l->activations);
    for (int i = 0; i < l->activations->rows * l->activations->cols; i++){
        l->activations->values[i] = (*l->activationFunc)(l->activations->values[i]);
    }
}




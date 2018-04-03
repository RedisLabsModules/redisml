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
    l->w= Matrix_New(size, inputSize);
    l->z = Matrix_New(size, inputSize);
    l->b= Matrix_New(size, 1);
    l->wGrad = Matrix_New(size, inputSize);
    l->bGrad = Matrix_New(size, 1);
    l->a= Matrix_New(size, 1);
    l->delta = Matrix_New(size, 1);
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
    Matrix_Multiply(input, l->w, l->z);
    Matrix_Add(l->a, l->b, l->z);
    for (int i = 0; i < l->a->rows * l->a->cols; i++){
        l->a->values[i] = (*l->activationFunc)(l->z->values[i]);
    }
}




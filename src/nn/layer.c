#include "layer.h"
#include "float.h"

float box_muller(){
    const float epsilon = FLT_MIN;
    const float two_pi = 2.0 * 3.14159265358979323846;
    static float z0, z1;
    static int generate;
    generate = generate == 1 ? 0 : 1;
    if (!generate){
        return z1;
    }
    float u1, u2;
    do{
        u1 = rand() * (1.0 / RAND_MAX);
        u2 = rand() * (1.0 / RAND_MAX);
    } while (u1 <= epsilon);
    z0 = sqrt(-2.0 * logf(u1)) * cos(two_pi * u2);
    z1 = sqrt(-2.0 * logf(u1)) * sin(two_pi * u2);
    return z0;
}



float Sigmoid(float x){
        return 1 / (1 + expf(-x));
}

float SigmoidPrime(float x){
    return Sigmoid(x) * (1 - Sigmoid(x));
}

Layer *Layer_Init(size_t size, size_t inputSize, LayerType type, ActivationType activationType) {
    printf("layer init\n");
    Layer *l = malloc(sizeof(Layer));
    l->type = type;
    l->activationType = activationType;
    l->size = size;
    l->inputSize = inputSize;
    l->w= Matrix_New(size, inputSize);
    l->z = Matrix_New(size, 1);
    l->b= Matrix_New(size, 1);
    l->wGrad = Matrix_New(size, inputSize);
    l->bGrad = Matrix_New(size, 1);
    l->a= Matrix_New(size, 1);
    l->wDelta = Matrix_New(size, inputSize);
    l->bDelta = Matrix_New(size, 1);
    l->delta = Matrix_New(size, 1);
    switch (activationType) {
        case SIGMOID:
            l->activationFunc = &Sigmoid;
            l->activationDerivativeFunc = &SigmoidPrime;
        default:
            l->activationFunc = &Sigmoid;
            l->activationDerivativeFunc = &SigmoidPrime;
    }
    
    float wsqrt = sqrt(l->w->cols);
    for (int i = 0; i < l->w->rows * l->w->cols; i++){
        l->w->values[i] =  box_muller()/wsqrt;
    }

    for (int i = 0; i < l->b->rows * l->b->cols; i++){
        l->b->values[i] = 0;
    }

    return l;
}

void Layer_CalcActivations(Layer *l, Matrix *input) {
    Matrix_Zeros(l->z);
    Matrix_Multiply(l->w, input, l->z);
    
    for (int i = 0; i < l->z->rows; i++) {
        l->z->values[i] += l->b->values[i];
    }
    
    for (int i = 0; i < l->a->rows; i++){
        l->a->values[i] = (*l->activationFunc)(l->z->values[i]);
    }
}




#include "layer.h"

float Sigmoid(float x){
        return 1 / (1 + expf(-1 * x));
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

    for (int i = 0; i < size * inputSize; i++){
        //l->w->values[i] = ((float)(i%10-5))/10.0;
        l->w->values[i] = 0.01f;
        //printf("%.3f\n",l->w->values[i]);
    }

    return l;
}

void Layer_CalcActivations(Layer *l, Matrix *input) {
    Matrix_Zeros(l->z);
    Matrix_Multiply(l->w, input, l->z);
    //printf("\nlayer calc activations: input(%zu,%zu), layer w(%zu,%zu), layer z(%zu,%zu) layer b(%zu,%zu)\n",input->rows,input->cols,l->w->rows,l->w->cols,l->z->rows,l->z->cols,l->b->rows,l->b->cols);

    for (int i = 0; i < l->z->rows; i++) {
        l->z->values[i] += l->b->values[i];
    }
    
    for (int i = 0; i < l->a->rows * l->a->cols; i++){
        l->a->values[i] = (*l->activationFunc)(l->z->values[i]);
    }
}




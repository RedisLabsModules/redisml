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

float SigmoidDeriv(float x){
    return Sigmoid(x) * (1 - Sigmoid(x));
}

float Relu(float x){
    return x > 0 ? x : 0;
}

float ReluDeriv(float x){
    return x > 0;
}

void Softmax(Layer *l){
    float sum = 0;
    for (int j=0; j < l->z->rows; j++){
        l->a->values[j] = expf(l->z->values[j]);
        sum += l->a->values[j];
    }
    for (int j=0; j < l->z->rows; j++){
        l->a->values[j] /= sum;
    }
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
            l->activationDerivativeFunc = &SigmoidDeriv;
            break;
        case RELU:
            l->activationFunc = &Relu;
            l->activationDerivativeFunc = &ReluDeriv;
            break;
        default:
            l->activationFunc = &Sigmoid;
            l->activationDerivativeFunc = &SigmoidDeriv;
            break;
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
    
    switch(l->activationType){
        case SIGMOID:
        case RELU:
            for (int i = 0; i < l->a->rows; i++){
                l->a->values[i] = (*l->activationFunc)(l->z->values[i]);
            }
            break;
        case SOFTMAX:
           Softmax(l);
           break; 
    }
}


void Layer_ConvCalcActivations(Layer *l, Matrix *input) {
    int frows = l->filters[0]->rows;
    int fcols = l->filters[0]->cols;
    int inrows = input->rows - l->padding - frows + 1;
    int incols = input->cols - l->padding - frows + 1;
    int in_x, in_y, out_x, out_y, f_x, f_y;
    Matrix *a;
    for (int f = 0; f < l->nfilters; f++){
        a = l->conv_a[f];
        for (in_x = l->padding; in_x < inrows; in_x += l->stride){
            out_x = (in_x - l->padding) / l->stride;
            for (in_y = l->padding; in_y < incols; in_y += l->stride){
                out_y = (in_y - l->padding) / l->stride;
                MATRIX(a, out_x, out_y) = 0;  
                for (f_x = 0; f_x < frows; f_x++){
                    for (f_y = 0; f_y < fcols; f_y++){
                        MATRIX(a, out_x, out_y) += MATRIX(input, in_x + f_x, in_y + f_y) * MATRIX(l->filters[f], f_x, f_y);  
                    }
                }
            }
        } 
    }
}


#include "network.h"

float CostSSE(float x, float y){
    return (x - y) * (x - y);
}

float CostSSEDeriv(float x, float y){
    return 2 * (x - y);
}

void NN_SGD(Network *n, size_t cycles, size_t batchSize, float rate){
    for (int c = 0; c < cycles; c++){
        for (int i = 0; i < n->trainingData->nSamples; i += batchSize){
            runMiniBatch(n, i, batchSize, rate);
        }
    }
}

void runMiniBatch(Network *n, int offset, size_t batchSize, float rate){
    //shortcut pointers
    Layer *l;
    Layer *lp;
    Layer *ln;

    for (int i = offset; i < offset + batchSize; i++){
        l = n->layers[n->nlayers - 1];
        lp = n->layers[n->nlayers - 2];

        feedForward(n, n->trainingData->features + i);
        //calculate delta for the last layer:
        for (int i = 0; i < l->delta->rows; i++){
            l->delta->values[i] = n->costDerivativeFunc(l->a->values[i], 0.1) * SigmoidPrime(l->z->values[i]);
            l->bGrad->values[i] = l->delta->values[i];
            l->bGrad->values[i] = l->delta->values[i] * lp->a->values[i];
        }

        //backprop through the rest of the layers
        for (int idx = n->nlayers - 2; idx >= 1; idx--){
            l = n->layers[idx];
            lp = n->layers[idx - 1];
            ln = n->layers[idx + 1];
            
            Matrix *wt = Matrix_New(ln->w->cols, ln->w->rows);
            Matrix_Transpose(ln->w, wt);
            Matrix_Multiply(wt, ln->delta, l->delta);
            for (int i = 0; i < l->delta->rows; i++){
                l->delta->values[i] *=  SigmoidPrime(l->z->values[i]);
                l->bGrad->values[i] = l->delta->values[i];
                l->bGrad->values[i] = l->delta->values[i] * lp->a->values[i];
            }
        }
    }
}

void backProp(Network *n, float *features, unsigned int label){
}

void feedForward(Network *n, float *features){
    Matrix input = {.cols = n->trainingData->featureSize, .rows = 1};
    input.values = features;
    Layer_CalcActivations(n->layers[0], &input);
    for (int i = 1; i < n->nlayers; n++){
        Layer_CalcActivations(n->layers[i], n->layers[i-1]->a);
    }
}



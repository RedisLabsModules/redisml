#include "network.h"

float CostSSE(float x, float y){
    return (x - y) * (x - y);
}

float CostSSEDeriv(float x, float y){
    return x - y;
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

        printf("feedforward\n");
        feedForward(n, n->trainingData->features + i);
        int class  = n->trainingData->labels[offset];
        //class = 1<<class;
        //printf("label: %u, class: %d\n", n->trainingData->labels[offset], class);
        //calculate delta for the last layer:
        printf("z:\n");
        Matrix_Print(l->z);
        printf("calc delta\n");
        for (int i = 0; i < l->bDelta->rows; i++){
            l->bDelta->values[i] = n->costDerivativeFunc(l->a->values[i], i==class?1:0) * SigmoidPrime(l->z->values[i]);
            //l->wDelta->values[i] = l->bDelta->values[i] * lp->a->values[i];
        }

        Matrix *lpat = Matrix_New(lp->a->cols, lp->a->rows);
        Matrix_Transpose(lp->a, lpat);

        Matrix_Multiply(l->bDelta, lpat, l->wDelta);
        printf("delta:\n");
        Matrix_Print(l->bDelta);
        printf("wDelta:\n");
        Matrix_Print(l->wDelta);
        //backprop through the rest of the layers
        printf("backprop\n");
        for (int idx = n->nlayers - 2; idx >= 1; idx--){
            l = n->layers[idx];
            lp = n->layers[idx - 1];
            ln = n->layers[idx + 1];
            
            Matrix *wt = Matrix_New(ln->w->cols, ln->w->rows);
            Matrix_Transpose(ln->w, wt);
            Matrix_Multiply(wt, ln->bDelta, l->bDelta);
            for (int i = 0; i < l->bDelta->rows; i++){
                l->bDelta->values[i] *=  SigmoidPrime(l->z->values[i]);
                l->wDelta->values[i] = l->bDelta->values[i] * lp->a->values[i];
            }
        }
    }
}


void feedForward(Network *n, float *features){
    n->layers[0]->a->values =  features;
    for (int i = 1; i < n->nlayers; i++){
        //Matrix_Zeros(n->layers[i]->z);
        //Matrix_Zeros(n->layers[i]->a);
        Layer_CalcActivations(n->layers[i], n->layers[i-1]->a);
    }
}



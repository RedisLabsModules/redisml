#include "network.h"

float CostSSE(float x, float y){
    return (x - y) * (x - y);
}

float CostSSEDeriv(float x, float y){
    return x - y;
}

void NN_SGD(Network *n, size_t cycles, size_t batchSize, float rate){
    Layer *l;
    for (int c = 0; c < cycles; c++){
        for (int i = 0; i < n->trainingData->nSamples; i += batchSize){
            runMiniBatch(n, i, batchSize, rate);
            for (int idx = 1; idx < n->nlayers; idx++){
                l = n->layers[idx];
                Matrix_Add(l->wGrad, l->wDelta, l->wGrad);
                Matrix_Add(l->bGrad, l->bDelta, l->bGrad);
                
                //w = w - (rate / batchSize)*wGrad
                for (int w = 0; w < l->w->cols * l->w->rows; w++){
                    l->w->values[w] -= l->wGrad->values[w]*rate/(float)batchSize;
                }
                for (int b = 0; b < l->b->cols * l->b->rows; b++){
                    l->b->values[b] -= l->bGrad->values[b]*rate/(float)batchSize;
                }
            } 
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

        feedForward(n, n->trainingData->features + i * n->trainingData->featureSize);
        int class  = n->trainingData->labels[i];
        //calculate delta for the last layer:
        for (int i = 0; i < l->delta->rows; i++){
            l->delta->values[i] = n->costDerivativeFunc(l->a->values[i], i==class?1:0) * SigmoidPrime(l->z->values[i]);
            l->bDelta->values[i] = l->delta->values[i];
        }

        Matrix *lpat = Matrix_New(lp->a->cols, lp->a->rows);
        Matrix_Transpose(lp->a, lpat);
        Matrix_Multiply(l->bDelta, lpat, l->wDelta);
        Matrix_Free(lpat);

        printf("bDeltaL:\n");
        Matrix_Print(l->bDelta);
        //backprop through the rest of the layers
        for (int idx = n->nlayers - 2; idx >= 1; idx--){
            l = n->layers[idx];
            lp = n->layers[idx - 1];
            ln = n->layers[idx + 1];
            
            Matrix *lntw = Matrix_New(ln->w->cols, ln->w->rows);
            Matrix_Transpose(ln->w, lntw);
            Matrix_Multiply(lntw, ln->delta, l->bDelta);
            Matrix_Free(lntw);
            for (int i = 0; i < l->bDelta->rows; i++){
                l->bDelta->values[i] *=  SigmoidPrime(l->z->values[i]);
            }
            Matrix *lpat2 = Matrix_New(lp->a->cols, lp->a->rows);
            Matrix_Transpose(lp->a, lpat2);
            Matrix_Multiply(l->bDelta, lpat2, l->wDelta);
            Matrix_Free(lpat2);
            printf("bDelta0:\n");
            Matrix_Print(l->bDelta);
        }
    }
}


void feedForward(Network *n, float *features){
    n->layers[0]->a->values =  features;
    for (int i = 1; i < n->nlayers; i++){
        Layer_CalcActivations(n->layers[i], n->layers[i-1]->a);
    }
}



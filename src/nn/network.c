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
            printf("w[2]:\n");
            Matrix_Print(n->layers[2]->w, 0);
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
        printf("******* START BP %d\n", i - offset);
        feedForward(n, &n->trainingData->features[i * n->trainingData->featureSize]);
        printf("activations:\n");
        Matrix_Print(l->a, 0);
        printf("z:\n");
        Matrix_Print(l->z, 5);
        printf("w:\n");
        Matrix_Print(l->w, 5);
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
        Matrix_Print(l->bDelta, 0);
        //backprop through the rest of the layers
        for (int idx = n->nlayers - 2; idx >= 1; idx--){
            l = n->layers[idx];
            lp = n->layers[idx - 1];
            ln = n->layers[idx + 1];
            
            Matrix *lntw = Matrix_New(ln->w->cols, ln->w->rows);
            Matrix_Transpose(ln->w, lntw);
            Matrix_Zeros(l->bDelta);
            Matrix_Multiply(lntw, ln->delta, l->bDelta);
            printf("z0:\n");
            Matrix_Print(l->z, 0);
            printf("w-l+1:\n");
            Matrix_Print(lntw, 0);
            Matrix_Free(lntw);
            printf("old delta:\n");
            Matrix_Print(ln->delta, 0);
            for (int i = 0; i < l->bDelta->rows; i++){
                l->bDelta->values[i] *=  SigmoidPrime(l->z->values[i]);
            }
            printf("new delta:\n");
            Matrix_Print(l->bDelta, 0);

            Matrix *lpat2 = Matrix_New(lp->a->cols, lp->a->rows);
            Matrix_Transpose(lp->a, lpat2);
            Matrix_Multiply(l->bDelta, lpat2, l->wDelta);
            Matrix_Free(lpat2);
        }
        printf("******* END BP %d\n", i - offset);
    }
}


void feedForward(Network *n, float *features){
    n->layers[0]->a->values =  features;
    for (int i = 1; i < n->nlayers; i++){
        Layer_CalcActivations(n->layers[i], n->layers[i-1]->a);
    }
}



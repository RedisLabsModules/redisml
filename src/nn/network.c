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
            for (int idx = 1; idx < n->nlayers; idx++){
                l = n->layers[idx];
                Matrix_Zeros(l->wGrad);
                Matrix_Zeros(l->bGrad);
            }
            runMiniBatch(n, i, batchSize, rate);
            for (int idx = 1; idx < n->nlayers; idx++){
                l = n->layers[idx];
                //w = w - (rate / batchSize)*wGrad
                for (int w = 0; w < l->w->cols * l->w->rows; w++){
                    l->w->values[w] -= l->wGrad->values[w]*rate/(float)batchSize;
                }
                for (int b = 0; b < l->b->cols * l->b->rows; b++){
                    l->b->values[b] -= l->bGrad->values[b]*rate/(float)batchSize;
                }
            } 
            //printf("w[2]:\n");
            //Matrix_Print(n->layers[2]->w, 0);
            //printf("b[2]:\n");
            //Matrix_Print(n->layers[2]->b, 0);
        }
#ifdef DEBUG        
        printf("dw[2]:\n");
        Matrix_Print(n->layers[2]->wDelta, 0);
        printf("db[2]:\n");
        Matrix_Print(n->layers[2]->bDelta, 0);
#endif        
        NN_Eval(n, 50000, 10000);
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
        //printf("******* START BP %d\n", i - offset);
        feedForward(n, &n->trainingData->features[i * n->trainingData->featureSize]);
        int class  = n->trainingData->labels[i];
        //calculate delta for the last layer:
        float sp = 0;
        float cd = 0;
        for (int i = 0; i < l->delta->rows; i++){
            sp = l->activationDerivativeFunc(l->z->values[i]);
            cd = n->costDerivativeFunc(l->a->values[i], i==class?1:0); 
            l->delta->values[i] = sp * cd;
            //printf("sp = %.12f, cd = %f\n", sp, cd);
            l->bDelta->values[i] = l->delta->values[i];
        }
#ifdef DEBUG
        printf("activations:\n");
        Matrix_Print(l->a, 0);
        printf("z:\n");
        Matrix_Print(l->z, 0);
        printf("w:\n");
        Matrix_Print(l->w, 0);
        printf("b:\n");
        Matrix_Print(l->b, 0);
        printf("delta:\n");
        Matrix_Print(l->delta, 0);
#endif
        Matrix *lpat = Matrix_New(lp->a->cols, lp->a->rows);
        Matrix_Transpose(lp->a, lpat);
#ifdef DEBUG
        printf("lpa:\n");
        Matrix_Print(lp->a, 0);
        printf("lpat:\n");
        Matrix_Print(lpat, 0);
#endif
        Matrix_Zeros(l->wDelta);
        Matrix_Multiply(l->bDelta, lpat, l->wDelta);
        Matrix_Free(lpat);
        /*
        int p = l->wDelta->rows * l->wDelta->cols -1;
        if (l->wDelta->values[p] != l->wDelta->values[p-1]){
            printf("exit on cycle: %d\n", i);
            exit(0);
        }
        */
#ifdef DEBUG
        printf("lpa diffs:\n");
        Matrix_PrintDiffs(lp->a);
        printf("lwdelta diffs:\n");
        Matrix_PrintDiffs(l->wDelta);
#endif
        //printf("bDeltaL:\n");
        //Matrix_Print(l->bDelta, 0);
        //backprop through the rest of the layers
        for (int idx = n->nlayers - 2; idx >= 1; idx--){
            l = n->layers[idx];
            lp = n->layers[idx - 1];
            ln = n->layers[idx + 1];
            
            Matrix *lntw = Matrix_New(ln->w->cols, ln->w->rows);
            Matrix_Transpose(ln->w, lntw);
            Matrix_Zeros(l->delta);
            Matrix_Multiply(lntw, ln->delta, l->delta);
            //printf("z0:\n");
            //Matrix_Print(l->z, 0);
            //printf("b0:\n");
            //Matrix_Print(l->b, 5);
            //printf("w-l+1:\n");
            //Matrix_Print(lntw, 0);
            Matrix_Free(lntw);
            //printf("old delta:\n");
            //Matrix_Print(ln->delta, 0);
            for (int i = 0; i < l->bDelta->rows; i++){
                l->delta->values[i] *=  l->activationDerivativeFunc(l->z->values[i]);
                l->bDelta->values[i] = l->delta->values[i];
            }
            //printf("new delta:\n");
            //Matrix_Print(l->bDelta, 0);

            Matrix *lpat2 = Matrix_New(lp->a->cols, lp->a->rows);
            Matrix_Transpose(lp->a, lpat2);
            Matrix_Zeros(l->wDelta);
            Matrix_Multiply(l->bDelta, lpat2, l->wDelta);
            Matrix_Free(lpat2);
        }
        for (int idx = 1; idx < n->nlayers; idx++){
            l = n->layers[idx];
            Matrix_Add(l->wGrad, l->wDelta, l->wGrad);
            Matrix_Add(l->bGrad, l->bDelta, l->bGrad);
            //printf("bGrad:\n");
            //Matrix_Print(l->bGrad, 0);
            //printf("wGrad:\n");
            //Matrix_Print(l->wGrad, 10);
        }
        //printf("******* END BP %d\n", i - offset);
    }
}


void feedForward(Network *n, float *features){
    memcpy(n->layers[0]->a->values, features, n->layers[0]->a->rows * sizeof(float));
    //n->layers[0]->a->values =  features;
    for (int i = 1; i < n->nlayers; i++){
        Layer_CalcActivations(n->layers[i], n->layers[i-1]->a);
    }
}

int getMaxActivation(Network *n){
    float maxVal = 0;
    float index = 0;
    Matrix *a = n->layers[n->nlayers - 1]->a;
    for (int i = 0; i < a->rows; i++){
        if (a->values[i] > maxVal){
            maxVal = a->values[i];
            index = i;
        }
    }
    return index;
}

void NN_Eval(Network *n, int start, int numSamples){
    int ok = 0;
    for (int i = start; i < start + numSamples; i++){
        feedForward(n, &n->trainingData->features[i * n->trainingData->featureSize]);
        if (n->trainingData->labels[i] == getMaxActivation(n)){
            ok++;
        }
    }
    printf("%d/%d ok.\n",ok,numSamples);
}





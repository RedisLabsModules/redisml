#include "network.h"

void NN_SGD(Network *n, size_t cycles, size_t batchSize, float rate){
    for (int c = 0; c < cycles; c++){
        for (int i = 0; i < n->trainingData->nSamples; i += batchSize){
            runMiniBatch(n, i, batchSize, rate);
        }
    }
}

void runMiniBatch(Network *n, int offset, size_t batchSize, float rate){
    for (int i = offset; i < offset + batchSize; i++){
        feedForward(n, n->trainingData->features + i);
        //calculate delta:
        n->layers[0]->activationDerivativeFunc(0.1); 
    }
}

void backProp(Network *n, float *features, unsigned int label){
}

void feedForward(Network *n, float *features){
    Matrix input = {.cols = n->trainingData->featureSize, .rows = 1};
    input.values = features;
    Layer_CalcActivations(n->layers[0], &input);
    for (int i = 1; i < n->nlayers; n++){
        Layer_CalcActivations(n->layers[i], n->layers[i-1]->activations);
    }
}



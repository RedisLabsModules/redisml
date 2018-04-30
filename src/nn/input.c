#include <stdio.h>
#include <stdlib.h>

#include "layer.h"
#include "network.h"

void printDigit(float *d){
    for (int i = 0; i < 784; i++){
        printf("%c", d[i]>0.5 ? '*' : ' ');
        if(i%28 == 0){
            printf("\n");
        }
    }
}

int getInt32(unsigned char *buff, FILE *f) {
    if(fread(buff, 4, 1, f))
        return (buff[0]<<24 | (buff[1]<<16) | (buff[2]<<8) |buff[3] );
        
    return 0;
}

int main (int argc, char**argv) {

    if (argc != 6) {
        printf("args: training_data, cycles, batch_size, learning_rate, num_layers\n");
        return 0;
    }

    char* trainingDataFile = argv[1];
    int cycles = atoi(argv[2]);
    int batchSize = atoi(argv[3]);
    float learningRate = atof(argv[4]);
    int nsamples = atoi(argv[5]);

    printf("trainingDataFile: %s\n", trainingDataFile);
    printf("cycles: %d\n", cycles);
    printf("batchSize: %d\n", batchSize);
    printf("learningRate: %.3f\n", learningRate);
    printf("nsamples: %d\n", nsamples);

    FILE *f;
    unsigned char buff[4];

    f = fopen(trainingDataFile,"r");
    if (f == NULL){
        printf("file missing\n");
    }

    int magic = getInt32(buff, f);
    printf("magic: %d\n", magic);

    int nimages = getInt32(buff, f);
    printf("nimages: %d\n", nimages);

    int rows = getInt32(buff, f);
    printf("rows: %d\n", rows);

    int cols = getInt32(buff, f);
    printf("cols: %d\n", cols);

    /* allocate training data */
    unsigned char* tmpTrainingData = calloc(nimages * rows * cols, sizeof(unsigned char));
    size_t rsize = fread(tmpTrainingData, 1, nimages * rows * cols, f);
    if (rsize != nimages * rows * cols){
        printf("error loading file to memory, read %zu bytes\n", rsize);
        return 0;
    }
    printf("success. read %zu bytes\n", rsize);
    float* trainingData = calloc(nimages * rows * cols, sizeof(float));
    for (int i = 0; i < nimages * rows * cols; i++){
        trainingData[i] = ((float)tmpTrainingData[i])/256.0;
    }
    free(tmpTrainingData);

    f = fopen("./data/mnist/train-labels-idx1-ubyte","r");
    if (f == NULL){
        printf("file missing\n");
    }

    magic = getInt32(buff, f);
    printf("labels magic: %d\n", magic);

    nimages = getInt32(buff, f);
    printf("nlabels: %d\n", nimages);

    /* allocate training labels */
    unsigned char* trainingLabels = calloc(nimages, sizeof(unsigned char));
    rsize = fread(trainingLabels, 1, nimages, f);
    if (rsize != nimages){
        printf("error loading file to memory, read %zu bytes\n", rsize);
        return 0;
    }
    fclose(f);
    /*
    int l = 50000;
    for (int i = (784 * l); i < ((50003) * 784); i+= 784){
        printf("\n\nlabel = %u, l=%d\n", trainingLabels[l], l);
        l++;
        printDigit(&trainingData[i]);
    }
    */
    
    
    /* init the network */
    Network *n = malloc(sizeof(Network));
    n->nlayers = 3;
    n->layers = malloc(n->nlayers * sizeof(Layer *));
    n->trainingData = malloc(sizeof(DataSet));
    //n->trainingData->nSamples = nimages/10;
    n->trainingData->nSamples = nsamples;
    n->trainingData->featureSize = rows * cols;
    n->trainingData->features = trainingData;
    n->trainingData->labels = trainingLabels;


    n->layers[0] = Layer_Init(rows * cols, 1,FULLY_CONNECTED, SIGMOID);
    n->layers[1] = Layer_Init(100, rows * cols, FULLY_CONNECTED, RELU);
    n->layers[2] = Layer_Init(10, 100, FULLY_CONNECTED, SOFTMAX);
    n->costDerivativeFunc = &CostSSEDeriv;
    
    /*
    printf("weights:\n");
    Matrix_Print(n->layers[2]->w, 0);    
    printf("biases:\n");
    Matrix_Print(n->layers[2]->b, 0);    
    printf("zzzzzz:\n");
    Matrix_Print(n->layers[2]->z, 0);    
    */
    //printf("a0:\n");
    //Matrix_Print(n->layers[0]->a, 0);    
    /*for (int i = 0; i < 5; i++){
        printf("\nactivation[%d]: \n", i);
        feedForward(n, &trainingData[i * 784]);
        //Matrix_Print(n->layers[0]->a, 0);    
        printf("a %d:\n",i);
        Matrix_Print(n->layers[2]->a, 0);    
    }*/

    NN_Eval(n, 50000, 10000);
    NN_SGD(n, cycles, batchSize, learningRate);
    /*
    NN_Eval(n, 10000);
    
    NN_SGD(n, 40, 50, 3.0f);
    NN_Eval(n, 10000);
    
    NN_SGD(n, 40, 50, 3.0f);
    NN_Eval(n, 5000);
    NN_SGD(n, 40, 50, 3.0f);
    NN_Eval(n, 5000);
    */

    //Matrix_Print(n->layers[0]->a, 0);    
    //printf("w 1:\n");
    //Matrix_Print(n->layers[1]->w, 0);    
    //printf("a 1:\n");
    //Matrix_Print(n->layers[1]->a, 0);    
    //printf("w 2:\n");
    //Matrix_Print(n->layers[2]->w, 0);    
    //printf("a 2:\n");
    //Matrix_Print(n->layers[2]->a, 0);   
    return(0);
}

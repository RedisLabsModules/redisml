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

/*
 * args: training_data, cycles, mini_batch_size, eta,                test_data=None):
 */
int main (int argc, char**argv) {

    if (argc != 6) {
        printf("args: training_data, cycles, batch_size, learning_rate, num_layers\n");
        return 0;
    }

    char* trainingDataFile = argv[1];
    int cycles = atoi(argv[2]);
    int batchSize = atoi(argv[3]);
    float learningRate = atof(argv[4]);
    int nlayers = atoi(argv[5]);

    printf("trainingDataFile: %s\n", trainingDataFile);
    printf("cycles: %d\n", cycles);
    printf("batchSize: %d\n", batchSize);
    printf("learningRate: %.3f\n", learningRate);
    printf("nlayers: %d\n", nlayers);

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
    int l = 0;
    for (int i = 0; i < 7840; i+= 784){
        printf("\n\nlabel = %u\n", trainingLabels[l++]);
        printDigit(&trainingData[i]);
    }
    fclose(f);
    
    
    
    /* init the network */
    Network *n = malloc(sizeof(Network));
    n->nlayers = 3;
    n->layers = malloc(n->nlayers * sizeof(Layer *));


    n->layers[0] = Layer_Init(rows * cols, 1,FULLY_CONNECTED, SIGMOID);
    n->layers[1] = Layer_Init(30, rows * cols, FULLY_CONNECTED, SIGMOID);
    n->layers[2] = Layer_Init(10, 30, FULLY_CONNECTED, SIGMOID);
    n->costDerivativeFunc = &CostSSEDeriv;

    printf("weights:\n");
    Matrix_Print(n->layers[2]->w);    
    printf("biases:\n");
    Matrix_Print(n->layers[2]->b);    
    printf("zzzzzz:\n");
    Matrix_Print(n->layers[2]->z);    
    //printf("a0:\n");
    //Matrix_Print(n->layers[0]->a);    
    for (int i = 0; i < 5; i++){
        printf("\nactivation[%d]: \n", i);
        feedForward(n, &trainingData[i * 784]);
        //Matrix_Print(n->layers[0]->a);    
        printf("a %d:\n",i);
        Matrix_Print(n->layers[2]->a);    
    }
    //Matrix_Print(n->layers[0]->a);    
    //printf("w 1:\n");
    //Matrix_Print(n->layers[1]->w);    
    //printf("a 1:\n");
    //Matrix_Print(n->layers[1]->a);    
    //printf("w 2:\n");
    //Matrix_Print(n->layers[2]->w);    
    //printf("a 2:\n");
    //Matrix_Print(n->layers[2]->a);    
    return(0);
}


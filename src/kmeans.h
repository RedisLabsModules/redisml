#ifndef __KMEANS_H__
#define __KMEANS_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define KMEANS_OK 0
#define KMEANS_ERR 1

typedef struct {
    int k;
    int dimensions;
    double *centers;
} Kmeans;

int KmeansPredict(double *features, Kmeans *);

#endif  // __KMEANS_H__

//

#include "kmeans.h"
#include "float.h"

int KmeansPredict(double *features, Kmeans *k) {
    double bestDist = DBL_MAX;
    int class = 0;
    int offset = 0;
    double dist = 0;
    for (int i = 0; i < k->k; i++) {
        dist = 0;
        offset = i * k->dimensions;
        for (int d = 0; d < k->dimensions; d++){
            dist += (features[d] - k->centers[offset + d]) * (features[d] - k->centers[offset + d]);
        }
        if (dist < bestDist) {
            bestDist = dist;
            class = i;
        }
    }
    return class;
}


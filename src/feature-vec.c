#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "feature-vec.h"

double FeatureVec_GetValue(FeatureVec *ir, char *key) {
    int nkey = (int) atof(key);
    if (nkey > 0 && nkey < MAX_NUM_FIELDS) {
        return ir->NumericFields[nkey];
    }
    for (int i = 0; i < ir->len; i++) {
        if (strcmp(key, ir->Features[i].key) == 0) {
            return ir->Features[i].value;
        }
    }
    return 0;
}

double FeatureVec_NumericGetValue(FeatureVec *ir, int nkey) {
    if (nkey > 0 && nkey < MAX_NUM_FIELDS) {
        return ir->NumericFields[nkey];
    }
    return 0;
}

void FeatureVec_Print(FeatureVec *ir) {
    printf("strings:\n");
    for (int i = 0; i < ir->len; i++) {
        printf("key: %s, val: %lf\n", ir->Features[i].key, ir->Features[i].value);
    }

    printf("numerics:\n");
    for (int i = 0; i < MAX_NUM_FIELDS; i++) {
        if (ir->NumericFields[i] > 0) {
            printf("key: %d, val: %lf\n", i, ir->NumericFields[i]);
        }
    }
}

int FeatureVec_Create(char *data, FeatureVec *fv) {
    char *start, *token, *saveptr = NULL, *subtoken1, *subtoken2,
            *subsaveptr = NULL;
    int fieldIndex = 0;
    start = strndup(data, strlen(data));
    token = strtok_r(start, ",", &saveptr);
    while (1) {
        subtoken1 = strtok_r(token, ":", &subsaveptr);
        subtoken2 = strtok_r(NULL, ":", &subsaveptr);
        if (subtoken1 == NULL || subtoken2 == NULL) {
            return 1;
        }
        /* try to add numeric field */
        int nkey = (int) atof(subtoken1);
        if (nkey > 0 && nkey < MAX_NUM_FIELDS) {
            fv->NumericFields[nkey] = atof(subtoken2);
        } else {
            fv->Features = realloc(fv->Features, sizeof(Feature) * (fieldIndex + 2));
            fv->Features[fieldIndex].key = strndup(subtoken1, strlen(subtoken1));
            fv->Features[fieldIndex].value = atof(subtoken2);
            fv->len++;
            fieldIndex++;
        }

        token = strtok_r(NULL, ",", &saveptr);
        if (token == NULL) {
            break;
        }
    }
    return 0;
}


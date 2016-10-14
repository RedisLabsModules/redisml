#ifndef __READER_H__
#define __READER_H__

#include <stdio.h>
#include <stdlib.h>

#define MAX_NUM_FIELDS 1024

typedef struct {
    char *key;
    double value;
} Field;

typedef struct {
    double Result;
    Field *Fields;
    size_t len;
    double NumericFields[MAX_NUM_FIELDS];
} InputRow;

void ReadInput(char *file, InputRow **rows);
int MakeInputRow(char *data, InputRow *ir);
void PrintInputRow(InputRow *ir);
double GetValue(InputRow *ir, char *key);
#endif  // __READER_H__
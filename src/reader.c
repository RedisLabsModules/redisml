#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "reader.h"

double GetValue(InputRow *ir, char *key) {
  int nkey = (int)atof(key);
  if (nkey > 0 && nkey < MAX_NUM_FIELDS) {
    return ir->NumericFields[nkey];
  }
  printf("BUG!, nkey=%d, key=%s\n", nkey, key);
  for (int i = 0; i < ir->len; i++) {
    if (strcmp(key, ir->Fields[i].key) == 0) {
      return ir->Fields[i].value;
    }
  }
  return 0;
}

void PrintInputRow(InputRow *ir) {
  printf("strings:\n");
  for (int i = 0; i < ir->len; i++) {
    printf("key: %s, val: %lf\n", ir->Fields[i].key, ir->Fields[i].value);
  }

  printf("numerics:\n");
  for (int i = 0; i < MAX_NUM_FIELDS; i++) {
    if (ir->NumericFields[i] > 0) {
      printf("key: %d, val: %lf\n", i, ir->NumericFields[i]);
    }
  }
}

int MakeInputRow(char *data, InputRow *ir) {
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
    // try to add numeric field
    int nkey = (int)atof(subtoken1);
    if (nkey > 0 && nkey < MAX_NUM_FIELDS) {
      ir->NumericFields[nkey] = atof(subtoken2);
    } else {
      ir->Fields = realloc(ir->Fields, sizeof(Field) * (fieldIndex + 2));
      ir->Fields[fieldIndex].key = strndup(subtoken1, strlen(subtoken1));
      ir->Fields[fieldIndex].value = atof(subtoken2);
      ir->len++;
      fieldIndex++;
    }

    token = strtok_r(NULL, ",", &saveptr);
    if (token == NULL) {
      break;
    }
  }
  return 0;
}

void ReadInput(char *file, InputRow **rows) {
  FILE *fp;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  char *token, *subtoken, *saveptr, *subsaveptr;
  int id, val;

  fp = fopen(file, "r");
  if (fp == NULL) exit(EXIT_FAILURE);

  read = getline(&line, &len, fp);
  printf("Retrieved line of length %zu :\n", read);
  printf("%s", line);

  token = strtok_r(line, " ", &saveptr);
  printf("token: %s\n", token);
  while (1) {
    token = strtok_r(NULL, " ", &saveptr);
    printf("token: %s\n", token);
    if (token == NULL) {
      break;
    }
    subtoken = strtok_r(token, ":", &subsaveptr);
    id = atoi(subtoken);
    subtoken = strtok_r(NULL, ":", &subsaveptr);
    val = atoi(subtoken);
    printf("token: %s, %d:%d\n", token, id, val);
  }

  free(line);
  return;
}

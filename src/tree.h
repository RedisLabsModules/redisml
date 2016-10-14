#ifndef __TREE_H__
#define __TREE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "reader.h"

#define TREE_OK 0
#define TREE_ERR 1

typedef enum {
    S_CATEGORICAL,
    S_NUMERICAL,
} SplitterType;

typedef enum {
    N_NODE,
    N_LEAF,
    N_EMPTY,
} NodeType;

typedef union {
    double fval;
    char *strval;
} SplitterVal;

typedef struct node {
    double predVal;
    NodeType type;
    char *splitterAttr;
    SplitterType splitterType;
    SplitterVal splitterVal;
    struct node *left;
    struct node *right;
} Node;

typedef struct { Node *root; } Tree;

typedef struct {
    size_t len;
    Tree **Trees;
} Forest;

Node *NewNode();
Node *NewNumericalNode(char *splitterAttr, double splitterVal);
Node *NewCategoricalNode(char *splitterAttr, char *splitterVal);
Node *NewLeaf(double);
int TreeAdd(Node **root, char *path, Node *n);
Node *TreeGet(Node *root, char *path);
void TreePrint(Node *root, char *path, int step);
void TreeDel(Node *root);
int TreeSerialize(char **dst, Node *root, char *path, int plen, int slen);
void TreeDeSerialize(char *s, Node **root, int slen);

double TreeClassify(InputRow *ir, Node *root);
void TreeTest();

#endif  // __TREE_H__

//

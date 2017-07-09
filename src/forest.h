#ifndef __FOREST_H__
#define __FOREST_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "feature-vec.h"
#include "util/thpool.h"

#define FOREST_OK 0
#define FOREST_ERR 1
#define FOREST_MAX_CLASSES 128

#define FOREST_USE_THREADS
//#define FOREST_USE_FAST_TREE

int FOREST_NUM_THREADS;
threadpool Forest_thpool;

typedef enum {
    S_CATEGORICAL,
    S_NUMERICAL,
} __forest_SplitterType;

typedef enum {
    N_NODE,
    N_LEAF,
    N_EMPTY,
} __forest_NodeType;

typedef union {
    double *fval;
    char *strval;
} __forest_SplitterVal;

typedef struct node {
    double predVal;
    __forest_NodeType type;
    char *splitterAttr;
    int numericSplitterAttr;
    __forest_SplitterType splitterType;
    __forest_SplitterVal splitterVal;
    int splitterValLen;
    double *stats;
    int statsLen;
    int statsTotal;
    struct node *left;
    struct node *right;
} __forest_Node;

typedef struct {
    int key;
    double val;
    int left;
    int right;
} __fast_Node;

typedef struct {
    __forest_Node *root;
    __fast_Node *fastTree;
    int numClasses;
    double *classCoefficients;
} Forest_Tree;

typedef struct {
    size_t len;
    Forest_Tree **Trees;
} Forest;

__forest_Node *Forest_NewNumericalNode(char *splitterAttr, double splitterVal);

__forest_Node *Forest_NewCategoricalNode(char *splitterAttr, char *splitterVal);

__forest_Node *Forest_NewLeaf(double, char*);

int Forest_TreeAdd(__forest_Node **root, char *path, __forest_Node *n);

void Forest_GenFastTree (Forest_Tree *t);

int Forest_CheckTree(Forest_Tree *t);

void Forest_NormalizeTree (Forest_Tree *t);

__forest_Node *Forest_TreeGet(__forest_Node *root, char *path);

void Forest_TreePrint(__forest_Node *root, char *path, int step);

void Forest_TreeDel(__forest_Node *root);

int Forest_TreeSerialize(char **dst, __forest_Node *root, char *path, int plen, int slen);

void Forest_TreeDeSerialize(char *s, __forest_Node **root, int slen);

__forest_Node *Forest_TreeClassify(FeatureVec *fv, __forest_Node *root);

double Forest_Classify(FeatureVec fv, Forest *f, int classification);

void Forest_TreeTest();

#endif  /* __FOREST_H__ */


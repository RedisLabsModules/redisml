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
    N_CATEGORICAL,
    N_NUMERICAL,
    N_LEAF,
    N_EMPTY,
} __forest_NodeType;

typedef struct node {
    __forest_NodeType type;
    int splitterAttr;
    double splitterVal;
    int statsLen;
    int *stats;
    int statsTotal;
    struct node *left;
    struct node *right;
} __forest_Node;

typedef struct {
    __forest_Node *root;
    //double *classCoefficients;
} Forest_Tree;

typedef struct {
    int nClasses;
    int nFeatures;
    size_t len;
    Forest_Tree **Trees;
} Forest;

__forest_Node *Forest_NewNode(__forest_NodeType type, int splitterAttr, double splitterVal, int* stats, int len);

int Forest_TreeAdd(__forest_Node **root, char *path, __forest_Node *n);

int Forest_CheckTree(Forest_Tree *t);

void Forest_NormalizeTree (Forest_Tree *t);

__forest_Node *Forest_TreeGet(__forest_Node *root, char *path);

void Forest_TreePrint(__forest_Node *root, char *path, int step);

void Forest_TreeDel(__forest_Node *root);

int Forest_TreeSerialize(char **dst, __forest_Node *root, char *path, int plen, int slen);

void Forest_TreeDeSerialize(char *s, __forest_Node **root, int slen);

__forest_Node *Forest_TreeClassify(double *fv, __forest_Node *root);

double Forest_Classify(FeatureVec fv, Forest *f, int classification);

void Forest_TreeTest();

#endif  /* __FOREST_H__ */


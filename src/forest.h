#ifndef __TREE_H__
#define __TREE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "feature-vec.h"

#define FOREST_OK 0
#define FOREST_ERR 1

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
    double fval;
    char *strval;
} __forest_SplitterVal;

typedef struct node {
    double predVal;
    __forest_NodeType type;
    char *splitterAttr;
    __forest_SplitterType splitterType;
    __forest_SplitterVal splitterVal;
    struct node *left;
    struct node *right;
} __forest_Node;

typedef struct {
    __forest_Node *root;
} Tree;

typedef struct {
    size_t len;
    Tree **Trees;
} Forest;

__forest_Node *Forest_NewNumericalNode(char *splitterAttr, double splitterVal);

__forest_Node *Forest_NewCategoricalNode(char *splitterAttr, char *splitterVal);

__forest_Node *Forest_NewLeaf(double);

int Forest_TreeAdd(__forest_Node **root, char *path, __forest_Node *n);

__forest_Node *Forest_TreeGet(__forest_Node *root, char *path);

void Forest_TreePrint(__forest_Node *root, char *path, int step);

void Forest_TreeDel(__forest_Node *root);

int Forest_TreeSerialize(char **dst, __forest_Node *root, char *path, int plen, int slen);

void Forest_TreeDeSerialize(char *s, __forest_Node **root, int slen);

double Forest_TreeClassify(FeatureVec *ir, __forest_Node *root);

void Forest_TreeTest();

#endif  /* __TREE_H__ */


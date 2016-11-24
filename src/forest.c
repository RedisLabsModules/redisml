
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "feature-vec.h"
#include "forest.h"


__forest_Node *__newNode(char *splitterAttr) {
    __forest_Node *n = (__forest_Node *) malloc(sizeof(__forest_Node));
    if (splitterAttr) {
        n->splitterAttr = malloc(strlen(splitterAttr) + 1);
        sprintf(n->splitterAttr, "%s", splitterAttr);
    }
    n->type = N_NODE;
    n->predVal = 0;
    n->left = NULL;
    n->right = NULL;
    return n;
}

__forest_Node *NewNumericalNode(char *splitterAttr, double sv) {
    __forest_Node *n = __newNode(splitterAttr);
    n->splitterType = S_NUMERICAL;
    n->splitterVal.fval = sv;
    return n;
}

__forest_Node *NewCategoricalNode(char *splitterAttr, char *sv) {
    __forest_Node *n = __newNode(splitterAttr);
    n->splitterType = S_CATEGORICAL;
    n->splitterVal.strval = malloc(strlen(sv) + 1);
    sprintf(n->splitterVal.strval, "%s", sv);
    return n;
}

__forest_Node *NewLeaf(double predVal) {
    __forest_Node *n = __newNode(NULL);
    n->type = N_LEAF;
    n->predVal = predVal;
    return n;
}

int Forest_TreeAdd(__forest_Node **root, char *path, __forest_Node *n) {
    if (strlen(path) == 1) {
        *root = n;
        return FOREST_OK;
    }

    if (*root == NULL) {
        printf("add node NULL ERR\n");
        return FOREST_ERR;
    }
    if (*(path + 1) == 'r') {
        return Forest_TreeAdd(&(*root)->right, path + 1, n);
    }
    if (*(path + 1) == 'l') {
        return Forest_TreeAdd(&(*root)->left, path + 1, n);
    }

    return FOREST_ERR;
}

__forest_Node *Forest_TreeGet(__forest_Node *root, char *path) {
    if (root == NULL) {
        return NULL;
    }

    if (strlen(path) == 1) {
        return root;
    }

    if (*(path + 1) == 'r') {
        return Forest_TreeGet(root->right, path + 1);
    }
    if (*(path + 1) == 'l') {
        return Forest_TreeGet(root->left, path + 1);
    }

    return NULL;
}

static void printNode(__forest_Node *n) {
    if (n->type == N_NODE) {
        if (n->splitterType == S_NUMERICAL) {
            printf("split=%s:%.2lf\n", n->splitterAttr, n->splitterVal.fval);
        } else if (n->splitterType == S_CATEGORICAL) {
            printf("split=%s:%s\n", n->splitterAttr, n->splitterVal.strval);
        }
    } else if (n->type == N_LEAF) {
        printf("pred=%.2lf\n", n->predVal);
    }
}

void Forest_TreePrint(__forest_Node *root, char *path, int step) {
    if (root != NULL) {
        printf("%d ==> %s:", step, path);
        printNode(root);
        char *nextpath = malloc(strlen(path) + 2);
        sprintf(nextpath, "%sl", path);
        Forest_TreePrint(root->left, nextpath, step + 1);
        sprintf(nextpath, "%sr", path);
        Forest_TreePrint(root->right, nextpath, step + 1);
    }
}

int Forest_TreeSerialize(char **dst, __forest_Node *root, char *path, int plen, int slen) {
    int len = slen;
    if (root == NULL) {
        return 0;
    }
    long int pathSize = plen / 8 + (plen % 8 ? 1 : 0);
    printf("******\npathSize: %ld\n", pathSize);
    size_t splitterSize = 0;
    size_t splitteAttrLen = 0;
    if (root->type != N_LEAF) {
        if (root->splitterType == S_NUMERICAL) {
            splitterSize = sizeof(double);
        } else if (root->splitterType == S_CATEGORICAL) {
            splitterSize = strlen(root->splitterVal.strval) + 1;
        }
        splitteAttrLen = strlen(root->splitterAttr) + 1 + sizeof(__forest_SplitterType);
    }

    len += sizeof(int) + pathSize + 1 + sizeof(double) + splitteAttrLen +
           splitterSize;
    printf("len: %d\n", len);
    *dst = realloc(*dst, len);
    char *s = *dst;
    printf("s: %p\n", s);
    int pos = slen;

    printf("plen: %d\n", plen);
    *((int *) (s + pos)) = plen;
    printf("s:plen: %d\n", *((int *) (s + pos)));
    pos += sizeof(int);

    if (plen) {
        *(s + pos) = *path;
        pos += pathSize;
    }

    *(s + pos) = (char) root->type;
    pos++;

    *((double *) (s + pos)) = root->predVal;
    pos += sizeof(double);

    printf("pos: %d, len: %d\n", pos, len);
    if (root->type != N_LEAF) {
        strcpy(s + pos, root->splitterAttr);
        pos += strlen(root->splitterAttr) + 1;

        *((__forest_SplitterType *) (s + pos)) = root->splitterType;
        pos += sizeof(__forest_SplitterType);

        if (root->splitterType == S_NUMERICAL) {
            *((double *) (s + pos)) = root->splitterVal.fval;
            pos += sizeof(double);
        } else if (root->splitterType == S_CATEGORICAL) {
            strcpy(s + pos, root->splitterVal.strval);
            pos += strlen(root->splitterVal.strval) + 1;
        }

        printf("pos: %d, len: %d\n", pos, len);
        char nextpath = *path << 1;
        len = Forest_TreeSerialize(dst, root->left, &nextpath, plen + 1, len);
        nextpath |= 1;
        len = Forest_TreeSerialize(dst, root->right, &nextpath, plen + 1, len);
    }
    return len;
}

void Forest_TreeDeSerialize(char *s, __forest_Node **root, int slen) {
    int pos = 0;
    while (pos < slen) {
        int len = *((int *) (s + pos));
        printf("*******************\npathlen: %d\n", len);
        pos += sizeof(int);
        char *path = malloc(len + 2);
        path[0] = '.';
        for (int i = 0; i < len; i++) {
            path[len - i] = (*(s + pos + i / 8) & (1 << (i % 8))) ? 'r' : 'l';
        }
        path[len + 1] = '\0';
        printf("destpath: %s\n", path);
        pos += len / 8 + (len % 8 ? 1 : 0);

        __forest_NodeType type = (__forest_NodeType) *(s + pos);
        printf("nodetype: %d\n", type);
        pos++;

        double predVal = *((double *) (s + pos));
        printf("predVal: %lf\n", predVal);
        pos += sizeof(double);

        __forest_Node *n = NULL;
        if (type != N_LEAF) {
            char *splitterAttr = s + pos;
            pos += strlen(splitterAttr) + 1;

            __forest_SplitterType splitterType = (__forest_SplitterType) *(s + pos);
            pos += sizeof(__forest_SplitterType);

            if (splitterType == S_NUMERICAL) {
                printf("numerical: %lf\n", *((double *) (s + pos)));
                n = NewNumericalNode(splitterAttr, *((double *) (s + pos)));
                pos += sizeof(double);
            } else if (splitterType == S_CATEGORICAL) {
                printf("categorical: %s\n", s + pos);
                n = NewCategoricalNode(splitterAttr, s + pos);
            }
        } else {
            n = NewLeaf(predVal);
        }
        Forest_TreeAdd(root, path, n);
        printf("last pos: %d\n", pos);
    }
}

void Forest_TreeDel(__forest_Node *root) {
    if (root != NULL) {
        Forest_TreeDel(root->left);
        Forest_TreeDel(root->right);
        root->left = NULL;
        root->right = NULL;
        free(root);
    }
}

double Forest_TreeClassify(FeatureVec *ir, __forest_Node *root) {
    if (root == NULL) {
        printf("Forest_TreeClassify reached NULL node\n");
        return 0;
    }
    if (root->type == N_LEAF) {
        return root->predVal;
    }

    double attrVal = GetValue(ir, root->splitterAttr);
    if (attrVal <= root->splitterVal.fval) {
        return Forest_TreeClassify(ir, root->left);
    }
    return Forest_TreeClassify(ir, root->right);
}

void Forest_TreeTest() {
    Tree t = {.root = NULL};
    __forest_Node *n1 = NewNumericalNode("1", 1.0);
    __forest_Node *n2 = NewNumericalNode("2", 2.0);
    __forest_Node *n3 = NewNumericalNode("3", 3.0);
    __forest_Node *n4 = NewNumericalNode("4", 4.0);

    __forest_Node *n5 = NewLeaf(5.0);
    __forest_Node *n6 = NewLeaf(6.0);
    __forest_Node *n7 = NewLeaf(7.0);
    __forest_Node *n8 = NewLeaf(8.0);
    __forest_Node *n9 = NewLeaf(9.0);

    Forest_TreeAdd(&t.root, ".", n1);
    Forest_TreeAdd(&t.root, ".l", n2);
    Forest_TreeAdd(&t.root, ".r", n3);
    Forest_TreeAdd(&t.root, ".rl", n8);
    Forest_TreeAdd(&t.root, ".rr", n9);
    Forest_TreeAdd(&t.root, ".ll", n4);
    Forest_TreeAdd(&t.root, ".lr", n7);
    Forest_TreeAdd(&t.root, ".lll", n5);
    Forest_TreeAdd(&t.root, ".llr", n6);

    printf("1***1\n");
    Forest_TreePrint(t.root, "+", 0);
    printf("2***2\n");
    char path = 0;
    char *s = NULL;
    printf("s: %p\n", s);
    int len = Forest_TreeSerialize(&s, t.root, &path, 0, 0);
    printf("slen: %d\n", len);
    printf("sizeof(__forest_Node): %ld\n", sizeof(__forest_Node));

    Tree dst = {.root = NULL};
    printf("s: %p\n", s);
    Forest_TreeDeSerialize(s, &dst.root, len);
    printf("3***3\n");
    Forest_TreePrint(dst.root, "+", 0);
    printf("Original:\n");
    Forest_TreePrint(t.root, "+", 0);

    return;
}

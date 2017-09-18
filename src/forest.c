#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "forest.h"
#include "util/logging.h"

__forest_Node *Forest_NewNode(__forest_NodeType type, int splitterAttr, double val, int *stats, int len) {
    __forest_Node *n = (__forest_Node *) malloc(sizeof(__forest_Node));
    n->left = NULL;
    n->right = NULL;
    n->stats = calloc(len, sizeof(int));
    memcpy(n->stats, stats, len);
    n->statsLen = len;
    n->type = type;
    n->splitterAttr = splitterAttr;
    n->splitterVal = val;
    return n;
}

int Forest_TreeAdd(__forest_Node **root, char *path, __forest_Node *n) {
    if (strlen(path) == 1) {
        *root = n;
        return FOREST_OK;
    }

    if (*root == NULL) {
        LG_DEBUG("add node NULL ERR\n");
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
    if (n->type != N_LEAF) {
            LG_DEBUG("split=%d:%.2lf\n", n->splitterAttr, n->splitterVal);
    } else if (n->type == N_LEAF) {
        LG_DEBUG("LEAF\n");
    }
}

void Forest_TreePrint(__forest_Node *root, char *path, int step) {
    if (root != NULL) {
        LG_DEBUG("%d ==> %s:", step, path);
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
    LG_DEBUG("******\npathSize: %ld\n", pathSize);
    size_t splitterSize = sizeof(double);
    size_t splitteAttrLen = sizeof(int);
    int statsLen = root->statsLen;
    int statsSize = statsLen * sizeof(int);

    len += sizeof(int) + pathSize + 1 + sizeof(double) + splitteAttrLen +
        splitterSize  + statsSize;
    LG_DEBUG("len: %d\n", len);
    *dst = realloc(*dst, len);
    char *s = *dst;
    LG_DEBUG("s: %p\n", s);
    int pos = slen;

    LG_DEBUG("plen: %d\n", plen);
    *((int *) (s + pos)) = plen;
    LG_DEBUG("s:plen: %d\n", *((int *) (s + pos)));
    pos += sizeof(int);

    if (plen) {
        *(s + pos) = *path;
        pos += pathSize;
    }

    *(s + pos) = (char) root->type;
    pos++;

    return len;
}

void Forest_TreeDeSerialize(char *s, __forest_Node **root, int slen) {
    int pos = 0;
    while (pos < slen) {
        int len = *((int *) (s + pos));
        LG_DEBUG("*******************\npathlen: %d\n", len);
        pos += sizeof(int);
        char *path = malloc(len + 2);
        path[0] = '.';
        for (int i = 0; i < len; i++) {
            path[len - i] = (*(s + pos + i / 8) & (1 << (i % 8))) ? 'r' : 'l';
        }
        path[len + 1] = '\0';
        LG_DEBUG("destpath: %s\n", path);
        pos += len / 8 + (len % 8 ? 1 : 0);

        __forest_NodeType type = (__forest_NodeType) *(s + pos);
        LG_DEBUG("nodetype: %d\n", type);
        pos++;

        double predVal = *((double *) (s + pos));
        LG_DEBUG("predVal: %lf\n", predVal);
        pos += sizeof(double);

        __forest_Node *n = NULL;
       Forest_TreeAdd(root, path, n);
        LG_DEBUG("last pos: %d\n", pos);
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

static int __checkTree(__forest_Node *root) {
    if (root == NULL) {
        return FOREST_ERR;
    }
    if (root->type == N_LEAF) {
        return FOREST_OK;
    }
    if (__checkTree(root->left) == FOREST_ERR){
        return FOREST_ERR;
    }
    if (__checkTree(root->right) == FOREST_ERR){
        return FOREST_ERR;
    }
    return FOREST_OK;
}


int Forest_CheckTree(Forest_Tree *t) {
    return __checkTree(t->root);
}

__forest_Node *Forest_TreeClassify(double *fv, __forest_Node *root) {
    if (root == NULL) {
        LG_DEBUG("Forest_TreeClassify reached NULL node\n");
        return NULL;
    }
    if (root->type == N_LEAF) {
        return root;
    }
    double attrVal;
    //    if (root->numericSplitterAttr) {
    //        attrVal = double_NumericGetValue(fv, root->numericSplitterAttr);
    //    } else {
    attrVal = double_GetValue(fv, root->splitterAttr);
    LG_DEBUG("splitter: %s,%lf len:%d", root->splitterAttr, attrVal, root->splitterValLen);
    for (int i = 0; i < root->splitterValLen; ++i) {
        LG_DEBUG("spliter[%d]: %lf", i, root->splitterVal.fval[i]);
    }
    //    }
    if (root->splitterType == S_NUMERICAL) {
        LG_DEBUG("numerical")
            if (attrVal <= root->splitterVal.fval[0]) {
                return Forest_TreeClassify(fv, root->left);
            }
        return Forest_TreeClassify(fv, root->right);
    } else {
        LG_DEBUG("categorical")
            for (int i = 0; i < root->splitterValLen; ++i) {
                if (attrVal == root->splitterVal.fval[i]) {
                    return Forest_TreeClassify(fv, root->left);
                }
            }
        return Forest_TreeClassify(fv, root->right);
    }
}

/*2D array comperator for the qsort*/
static int cmp2D(const void *p1, const void *p2) {
    return (int) (((const double *) p2)[1] - ((const double *) p1)[1]);
}

typedef struct {
    double *res;
    Forest *f;
    double *fv;
    int skip;
} taskArgs;

taskArgs ta;

static void classifierTask(void *id){
    int tid = *(int *)id; 
    int resOffset = tid * FOREST_MAX_CLASSES;
    int tStart = tid * ta.skip;
    int tEnd = tStart + ta.skip > ta.f->len ? ta.f->len : tStart + ta.skip;
    for (int i = tStart; i < (int) tEnd; i++) {
        __forest_Node *n = Forest_TreeClassify(ta.fv, ta.f->Trees[i]->root);
        for (int j=0; j < n->statsLen; j++){
            ta.res[resOffset + j] += n->stats == NULL ? 1: n->stats[j]/n->statsTotal;
        }
    }
}

/*Classify a feature vector with a full forest.
 * classification: majority voting of the trees.
 * regression: avg of the votes.*/
double Forest_Classify(double fv, Forest *f, int classification) {
    double threadResults[FOREST_MAX_CLASSES * FOREST_NUM_THREADS];
    memset(threadResults, 0, FOREST_MAX_CLASSES * FOREST_NUM_THREADS * sizeof(double));
    double results[FOREST_MAX_CLASSES][2] = {{0}};
    int tids[FOREST_NUM_THREADS];
    double rep = 0;
    
    ta.res = &threadResults[0];
    ta.fv = &fv;
    ta.f = f;
    ta.skip = (f->len + 1) / FOREST_NUM_THREADS; 

    if (classification) {
        LG_DEBUG("\nclassification:");
        for (int i = 0; i < FOREST_NUM_THREADS; i++){
            tids[i] = i;
            thpool_add_work(Forest_thpool, (void*)classifierTask, (void *)&tids[i]);
        };
        thpool_wait(Forest_thpool);
        for (int i = 0; i < FOREST_NUM_THREADS; i++) {
            for (int j=0; j < FOREST_MAX_CLASSES; j++){
                // printf("%.2lf,", n->stats[j]);
                results[j][0] = (double) j;
                results[j][1] += threadResults[i * FOREST_MAX_CLASSES + j];
            }
        }
        qsort(&results[0], FOREST_MAX_CLASSES, sizeof(results[0]), cmp2D);
        rep = results[0][0];
        for (int i = 0; i < 10; ++i) {
            // printf("results[%d]: %d,%.2lf,  th:%.2lf\n", i, (int)(results[i][0]), results[i][1], threadResults[i]);
        }
    } else {
        LG_DEBUG("regression");
        for (int i = 0; i < (int) f->len; i++) {
            rep += Forest_TreeClassify(&fv, f->Trees[i]->root)->predVal;
        }
        rep /= (int) f->len;
    }
    return rep;
}

int __genSubTreeNodeIndex;

static void __forest_genSubTree(__forest_Node **root, char *path, int depth) {
    __forest_Node *n;
    char *lpath = malloc(strlen(path) + 2);
    sprintf(lpath, "%sl", path);
    char *rpath = malloc(strlen(path) + 2);
    sprintf(rpath, "%sr", path);

    double val = (double) rand() / RAND_MAX;
    if (depth <= 0) {
        n = Forest_NewLeaf(val, "");
        Forest_TreeAdd(root, path, n);
        return;
    }
    char key[32];
    sprintf(key, "%d", __genSubTreeNodeIndex++);
    n = Forest_NewNumericalNode(key, val);
    Forest_TreeAdd(root, path, n);
    __forest_genSubTree(root, lpath, depth - 1);
    __forest_genSubTree(root, rpath, depth - 1);
}

void Forest_TreeTest() {
    struct timespec end, start;
    double ms;
    clock_gettime(CLOCK_REALTIME, &start);
    Forest f = {.len = 100};
    f.Trees = malloc(f.len * sizeof(Forest_Tree *));
    for (int i = 0; i < f.len; i++) {
        //LG_DEBUG("%d\n", i);
        Forest_Tree *t = malloc(sizeof(Forest_Tree));
        __forest_genSubTree(&t->root, ".", 8);
        //Forest_TreePrint(t->root, "+", 0);
        Forest_GenFastTree(t);
        f.Trees[i] = t;
    }

    clock_gettime(CLOCK_REALTIME, &end);

    ms =
        ((double) (end.tv_sec * 1.0e3 - start.tv_sec * 1.0e3) +
         (double) (end.tv_nsec - start.tv_nsec) / 1.0e6);
    LG_DEBUG("create elapsed: %lf\n", ms);

    double fv = {0, NULL};
    char *data = "1:0,2:1,3:0";
    double_Create(data, &fv);
    clock_gettime(CLOCK_REALTIME, &start);

    Forest_Classify(fv, &f, 1);
    clock_gettime(CLOCK_REALTIME, &end);

    ms =
        ((double) (end.tv_sec * 1.0e3 - start.tv_sec * 1.0e3) +
         (double) (end.tv_nsec - start.tv_nsec) / 1.0e6);
    LG_DEBUG("classify elapsed: %lf\n", ms);

    return;

    Forest_Tree t = {.root = NULL};
    __forest_genSubTree(&t.root, ".", 6);
    LG_DEBUG("1***1\n");
    Forest_TreePrint(t.root, "+", 0);
    LG_DEBUG("2***2\n");


    __forest_Node *n1 = Forest_NewNumericalNode("1", 1.0);
    __forest_Node *n2 = Forest_NewNumericalNode("2", 2.0);
    __forest_Node *n3 = Forest_NewNumericalNode("3", 3.0);
    __forest_Node *n4 = Forest_NewNumericalNode("4", 4.0);

    __forest_Node *n5 = Forest_NewLeaf(5.0, "");
    __forest_Node *n6 = Forest_NewLeaf(6.0, "");
    __forest_Node *n7 = Forest_NewLeaf(7.0, "");
    __forest_Node *n8 = Forest_NewLeaf(8.0, "");
    __forest_Node *n9 = Forest_NewLeaf(9.0, "");

    Forest_TreeAdd(&t.root, ".", n1);
    Forest_TreeAdd(&t.root, ".l", n2);
    Forest_TreeAdd(&t.root, ".r", n3);
    Forest_TreeAdd(&t.root, ".rl", n8);
    Forest_TreeAdd(&t.root, ".rr", n9);
    Forest_TreeAdd(&t.root, ".ll", n4);
    Forest_TreeAdd(&t.root, ".lr", n7);
    Forest_TreeAdd(&t.root, ".lll", n5);
    Forest_TreeAdd(&t.root, ".llr", n6);

    LG_DEBUG("1***1\n");
    Forest_TreePrint(t.root, "+", 0);
    LG_DEBUG("2***2\n");
    char path = 0;
    char *s = NULL;
    LG_DEBUG("s: %p\n", s);
    int len = Forest_TreeSerialize(&s, t.root, &path, 0, 0);
    LG_DEBUG("slen: %d\n", len);
    LG_DEBUG("sizeof(__forest_Node): %ld\n", sizeof(__forest_Node));

    Forest_Tree dst = {.root = NULL};
    LG_DEBUG("s: %p\n", s);
    Forest_TreeDeSerialize(s, &dst.root, len);
    LG_DEBUG("3***3\n");
    Forest_TreePrint(dst.root, "+", 0);
    LG_DEBUG("Original:\n");
    Forest_TreePrint(t.root, "+", 0);

    return;
}

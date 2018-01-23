#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "feature-vec.h"
#include "forest.h"
#include "util/logging.h"

__forest_Node *__newNode(char *splitterAttr) {
    __forest_Node *n = (__forest_Node *) malloc(sizeof(__forest_Node));
    if (splitterAttr) {
        n->splitterAttr = malloc(strlen(splitterAttr) + 1);
        sprintf(n->splitterAttr, "%s", splitterAttr);
        n->numericSplitterAttr = atoi(splitterAttr);
    }
    n->type = N_NODE;
    n->predVal = 0;
    n->splitterValLen = 0;
    n->left = NULL;
    n->right = NULL;
    n->stats = NULL;
    return n;
}

__forest_Node *Forest_NewNumericalNode(char *splitterAttr, double sv) {
    __forest_Node *n = __newNode(splitterAttr);
    n->splitterType = S_NUMERICAL;
    n->splitterVal.fval = malloc(sizeof(double));
    n->splitterVal.fval[0] = sv;
    return n;
}

__forest_Node *Forest_NewCategoricalNode(char *splitterAttr, char *sv) {
    __forest_Node *n = __newNode(splitterAttr);
    n->splitterType = S_CATEGORICAL;
    int len = 0;
    char *str = strdup(sv);
    char *s2 = strdup(sv);
    char *v;
    while ((v = strsep(&str, ":"))) len++;
    LG_DEBUG("adding splitter: %s, len: %d", sv, len);
    n->splitterVal.fval = calloc(len, sizeof(double));
    len = 0;
    while ((v = strsep(&s2, ":"))) {
        n->splitterVal.fval[len] = atof(v);
        LG_DEBUG("fval[%d]: %lf", len, n->splitterVal.fval[len]);
        len++;
    }
    n->splitterValLen = len;
    return n;
}

__forest_Node *Forest_NewLeaf(double predVal, char* stats) {
    __forest_Node *n = __newNode(NULL);
    n->type = N_LEAF;
    int len = 0;
    int total = 0;
    if(stats){
        char *str = strdup(stats);
        char *s2 = strdup(stats);
        char *v;
        while ((v = strsep(&str, ":"))) {
            len++;
            total += atoi(v);
        }
        LG_DEBUG("adding stats: %s, len: %d", stats, len);
        n->stats = calloc(len, sizeof(double));
        len = 0;
        while ((v = strsep(&s2, ":"))) {
            n->stats[len] = (double)atoi(v);
            LG_DEBUG("stats[%d]: %.3lf", len, n->stats[len]);
            len++;
        }
    }else{
        len = (int)predVal + 1;
        total = 1; 
        n->stats = calloc(len ,sizeof(double));
        n->stats[len-1] = 1;
    }
    n->statsTotal = total;
    n->statsLen = len;
    n->predVal = predVal;
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

static int fastIndex;

static void __forest_GenFastTree(__forest_Node *root, Forest_Tree *t, int parent, int isLeft) {
    if (root != NULL) {
        fastIndex++;
        int nextParent = fastIndex;
        t->fastTree[fastIndex].key = root->numericSplitterAttr;
        t->fastTree[fastIndex].val = root->splitterVal.fval[0];
        t->fastTree[fastIndex].left = -1;
        t->fastTree[fastIndex].right = -1;
        //        LG_DEBUG("index: %d\n", fastIndex);
        if (parent >= 0) {
            if (isLeft) {
                t->fastTree[parent].left = fastIndex;
                //                LG_DEBUG("t[%d].left = %d\n", parent, fastIndex);
            } else {
                t->fastTree[parent].right = fastIndex;
                //                LG_DEBUG("t[%d].right = %d\n", parent, fastIndex);
            }
        }
        __forest_GenFastTree(root->left, t, nextParent, 1);
        __forest_GenFastTree(root->right, t, nextParent, 0);
    }
}

void Forest_GenFastTree(Forest_Tree *t) {
    //    LG_DEBUG("gen_fast\n");
    fastIndex = -1;
    t->fastTree = malloc(sizeof(__fast_Node) * 1024);
    __forest_GenFastTree(t->root, t, -1, 0);
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
            LG_DEBUG("split=%s:%.2lf\n", n->splitterAttr, n->splitterVal.fval[0]);
        } else if (n->splitterType == S_CATEGORICAL) {
            LG_DEBUG("split=%s:%s\n", n->splitterAttr, n->splitterVal.strval);
        }
    } else if (n->type == N_LEAF) {
        LG_DEBUG("pred=%.2lf\n", n->predVal);
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
    size_t splitterSize = 0;
    size_t splitteAttrLen = 0;
    size_t statsLen = 0;
    size_t statsSize = 0;
    if (root->type != N_LEAF) {
        if (root->splitterType == S_NUMERICAL) {
            splitterSize = sizeof(double);
        } else if (root->splitterType == S_CATEGORICAL) {
            splitterSize = strlen(root->splitterVal.strval) + 1;
        }
        splitteAttrLen = strlen(root->splitterAttr) + 1 + sizeof(__forest_SplitterType);
    } else {
        statsLen = root->statsLen;
        statsSize = statsLen * sizeof(double) + sizeof(int) + sizeof(int);
    }

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
//        *(s + pos) = *path;
        for (int i=0; i<pathSize; i++) {
            *(s + pos + i) = *(path + i);
        }
        pos += pathSize;
    }

    *(s + pos) = (char) root->type;
    pos++;

    *((double *) (s + pos)) = root->predVal;
    pos += sizeof(double);

    LG_DEBUG("pos: %d, len: %d\n", pos, len);
    if (root->type != N_LEAF) {
        strcpy(s + pos, root->splitterAttr);
        pos += strlen(root->splitterAttr) + 1;

        *((__forest_SplitterType *) (s + pos)) = root->splitterType;
        pos += sizeof(__forest_SplitterType);

        if (root->splitterType == S_NUMERICAL) {
            *((double *) (s + pos)) = root->splitterVal.fval[0];
            pos += sizeof(double);
        } else if (root->splitterType == S_CATEGORICAL) {
            strcpy(s + pos, root->splitterVal.strval);
            pos += strlen(root->splitterVal.strval) + 1;
        }

        LG_DEBUG("pos: %d, len: %d\n", pos, len);

        // if we'll need another byte for the next path, increment pathSize
        if (plen > 0 && plen % 8 == 0) {
            pathSize += 1;
        }
        char *nextpath = malloc(pathSize);
        for (int i=0; i<pathSize; i++) {
            *(nextpath + i) = *(path + i) << 1;
            if (i > 0) {
                *(nextpath + i) |= (*(path + i - 1) & 0x80) >> 7;
            }
        }
        len = Forest_TreeSerialize(dst, root->left, nextpath, plen + 1, len);
        if (len == 0) {
            LG_DEBUG("left 0 len, pathlen = %d: ", plen);
            char printpath = *path;
            for (int i=0; i<plen; i++) {
              LG_DEBUG(printpath & 1 ? "r" : "l");
              printpath = printpath >> 1;
            }
        }
        *nextpath |= 1;
        len = Forest_TreeSerialize(dst, root->right, nextpath, plen + 1, len);
        if (len == 0) {
            LG_DEBUG("right 0 len, pathlen = %d: ", plen);
            char printpath = *path;
            for (int i=0; i<plen; i++) {
              LG_DEBUG(printpath & 1 ? "r" : "l");
              printpath = printpath >> 1;
            }
        }
        free(nextpath);
    } else {
        *((int *) (s + pos)) = root->statsLen;
        pos += sizeof(int);
        *((int *) (s + pos)) = root->statsTotal;
        pos += sizeof(int);
        for(int i=0; i < statsLen; i++){
            *((double *) (s + pos)) = root->stats[i];
            pos += sizeof(double);
        }
    }
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
        if (type != N_LEAF) {
            char *splitterAttr = s + pos;
            pos += strlen(splitterAttr) + 1;

            __forest_SplitterType splitterType = (__forest_SplitterType) *(s + pos);
            pos += sizeof(__forest_SplitterType);

            if (splitterType == S_NUMERICAL) {
                LG_DEBUG("numerical: %lf\n", *((double *) (s + pos)));
                n = Forest_NewNumericalNode(splitterAttr, *((double *) (s + pos)));
                pos += sizeof(double);
            } else if (splitterType == S_CATEGORICAL) {
                LG_DEBUG("categorical: %s\n", s + pos);
                n = Forest_NewCategoricalNode(splitterAttr, s + pos);
                pos += strlen(s + pos) + 1;
            }
        } else {
            n = Forest_NewLeaf(predVal, "");
            n->statsLen = *((int *) (s + pos));
            pos += sizeof(int);
            n->statsTotal = *((int *) (s + pos));
            pos += sizeof(int);
            n->stats = calloc(n->statsLen, sizeof(double));
            for(int i = 0; i < n->statsLen; i++){
                n->stats[i] = *((double *) (s + pos));
                pos += sizeof(double);
            }
        }
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

static void __normalizeTree(__forest_Node *root, Forest_Tree *t) {
    if (root->type == N_LEAF) {
        t->classCoefficients[(int) root->predVal]++;
        t->numClasses++;
        return;
    }
    __normalizeTree(root->left, t);
    __normalizeTree(root->right, t);
}

void Forest_NormalizeTree(Forest_Tree *t) {
    t->numClasses = 0;
    t->classCoefficients = calloc(FOREST_MAX_CLASSES, sizeof(double));
    __normalizeTree(t->root, t);
    for (int i = 0; i < FOREST_MAX_CLASSES; ++i) {
        t->classCoefficients[i] /= t->numClasses;
    }
}


__forest_Node *Forest_TreeClassify(FeatureVec *fv, __forest_Node *root) {
    if (root == NULL) {
        LG_DEBUG("Forest_TreeClassify reached NULL node\n");
        return NULL;
    }
    if (root->type == N_LEAF) {
        LG_DEBUG("leaf: %lf", root->predVal);
        return root;
    }
    double attrVal;
    //    if (root->numericSplitterAttr) {
    //        attrVal = FeatureVec_NumericGetValue(fv, root->numericSplitterAttr);
    //    } else {
    attrVal = FeatureVec_GetValue(fv, root->splitterAttr);
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


double Forest_FastTreeClassify(FeatureVec *fv, Forest_Tree *t) {
    //    LG_DEBUG("fast_classify\n");
    int i = 0;
    while (t->fastTree[i].left > 0 || t->fastTree[i].right > 0) {
        double attrVal;
        attrVal = FeatureVec_NumericGetValue(fv, t->fastTree[i].key);
        //        LG_DEBUG("i=%lf\n",attrVal);
        if (attrVal <= t->fastTree[i].val) {
            i = t->fastTree[i].left;
        } else {
            i = t->fastTree[i].right;
        }
        //        LG_DEBUG("i=%d\n",i);
    }
    return t->fastTree[i].val;
}

/*2D array comperator for the qsort*/
static int cmp2D(const void *p1, const void *p2) {
    return (int) (((const double *) p2)[1] - ((const double *) p1)[1]);
}

typedef struct {
    double *res;
    Forest *f;
    FeatureVec *fv;
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
double Forest_Classify(FeatureVec fv, Forest *f, int classification) {
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

int Forest_TreeTest() {
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

    FeatureVec fv = {0, NULL};
    char *data = "1:0,2:1,3:0";
    FeatureVec_Create(data, &fv);
    clock_gettime(CLOCK_REALTIME, &start);

    Forest_Classify(fv, &f, 1);
    clock_gettime(CLOCK_REALTIME, &end);

    ms =
        ((double) (end.tv_sec * 1.0e3 - start.tv_sec * 1.0e3) +
         (double) (end.tv_nsec - start.tv_nsec) / 1.0e6);
    LG_DEBUG("classify elapsed: %lf\n", ms);

    return FOREST_OK;
}

int Forest_DeepTreeTest() {

    Forest_Tree t = {.root = NULL};
    __forest_genSubTree(&t.root, ".", 6);
    LG_DEBUG("1***1\n");
    Forest_TreePrint(t.root, "+", 0);
    LG_DEBUG("2***2\n");


    __forest_Node *n1 = Forest_NewNumericalNode("n1", 1.0);
    __forest_Node *n2 = Forest_NewNumericalNode("n2", 2.0);
    __forest_Node *n3 = Forest_NewNumericalNode("n3", 3.0);
    __forest_Node *n4 = Forest_NewNumericalNode("n4", 4.0);
    __forest_Node *n5 = Forest_NewNumericalNode("n5", 5.0);
    __forest_Node *n6 = Forest_NewNumericalNode("n6", 6.0);
    __forest_Node *n7 = Forest_NewNumericalNode("n7", 7.0);
    __forest_Node *n8 = Forest_NewNumericalNode("n8", 8.0);
    __forest_Node *n9 = Forest_NewNumericalNode("n9", 9.0);
    __forest_Node *n10 = Forest_NewNumericalNode("n10", 10.0);
    __forest_Node *n11 = Forest_NewNumericalNode("n11", 11.0);
    __forest_Node *n12 = Forest_NewNumericalNode("n12", 12.0);
    __forest_Node *n13 = Forest_NewNumericalNode("n13", 13.0);
    __forest_Node *n14 = Forest_NewNumericalNode("n14", 14.0);
    __forest_Node *n15 = Forest_NewNumericalNode("n15", 15.0);
    __forest_Node *n16 = Forest_NewNumericalNode("n16", 16.0);
    __forest_Node *n17 = Forest_NewNumericalNode("n17", 17.0);
    __forest_Node *n18 = Forest_NewNumericalNode("n18", 18.0);
    __forest_Node *n19 = Forest_NewNumericalNode("n19", 19.0);

    __forest_Node *l1 = Forest_NewLeaf(1.0, "");
    __forest_Node *l2 = Forest_NewLeaf(2.0, "");
    __forest_Node *l3 = Forest_NewLeaf(3.0, "");
    __forest_Node *l4 = Forest_NewLeaf(4.0, "");
    __forest_Node *l5 = Forest_NewLeaf(5.0, "");
    __forest_Node *l6 = Forest_NewLeaf(6.0, "");
    __forest_Node *l7 = Forest_NewLeaf(7.0, "");
    __forest_Node *l8 = Forest_NewLeaf(8.0, "");
    __forest_Node *l9 = Forest_NewLeaf(9.0, "");
    __forest_Node *l10 = Forest_NewLeaf(10.0, "");
    __forest_Node *l11 = Forest_NewLeaf(11.0, "");
    __forest_Node *l12 = Forest_NewLeaf(12.0, "");
    __forest_Node *l13 = Forest_NewLeaf(13.0, "");
    __forest_Node *l14 = Forest_NewLeaf(14.0, "");
    __forest_Node *l15 = Forest_NewLeaf(15.0, "");
    __forest_Node *l16 = Forest_NewLeaf(16.0, "");
    __forest_Node *l17 = Forest_NewLeaf(17.0, "");
    __forest_Node *l18 = Forest_NewLeaf(18.0, "");
    __forest_Node *l19 = Forest_NewLeaf(19.0, "");
    __forest_Node *l20 = Forest_NewLeaf(20.0, "");

    Forest_TreeAdd(&t.root, ".", n1);
    Forest_TreeAdd(&t.root, ".l", n2);
    Forest_TreeAdd(&t.root, ".r", n3);
    Forest_TreeAdd(&t.root, ".ll", l2);
    Forest_TreeAdd(&t.root, ".lr", l3);
    Forest_TreeAdd(&t.root, ".rl", l1);
    Forest_TreeAdd(&t.root, ".rr", n4);
    Forest_TreeAdd(&t.root, ".rrr", n5);
    Forest_TreeAdd(&t.root, ".rrl", l4);
    Forest_TreeAdd(&t.root, ".rrrr", n6);
    Forest_TreeAdd(&t.root, ".rrrl", l5);
    Forest_TreeAdd(&t.root, ".rrrrr", n7);
    Forest_TreeAdd(&t.root, ".rrrrl", l6);
    Forest_TreeAdd(&t.root, ".rrrrrr", n8);
    Forest_TreeAdd(&t.root, ".rrrrrl", l7);
    Forest_TreeAdd(&t.root, ".rrrrrrr", n9);
    Forest_TreeAdd(&t.root, ".rrrrrrl", l8);
    Forest_TreeAdd(&t.root, ".rrrrrrrr", n10);
    Forest_TreeAdd(&t.root, ".rrrrrrrl", l9);
    Forest_TreeAdd(&t.root, ".rrrrrrrrr", n11);
    Forest_TreeAdd(&t.root, ".rrrrrrrrl", l10);
    Forest_TreeAdd(&t.root, ".rrrrrrrrrr", n12);
    Forest_TreeAdd(&t.root, ".rrrrrrrrrl", l11);
    Forest_TreeAdd(&t.root, ".rrrrrrrrrrr", n13);
    Forest_TreeAdd(&t.root, ".rrrrrrrrrrl", l12);
    Forest_TreeAdd(&t.root, ".rrrrrrrrrrrr", n14);
    Forest_TreeAdd(&t.root, ".rrrrrrrrrrrl", l13);
    Forest_TreeAdd(&t.root, ".rrrrrrrrrrrrr", n15);
    Forest_TreeAdd(&t.root, ".rrrrrrrrrrrrl", l14);
    Forest_TreeAdd(&t.root, ".rrrrrrrrrrrrrr", n16);
    Forest_TreeAdd(&t.root, ".rrrrrrrrrrrrrl", l15);
    Forest_TreeAdd(&t.root, ".rrrrrrrrrrrrrrr", n17);
    Forest_TreeAdd(&t.root, ".rrrrrrrrrrrrrrl", l16);
    Forest_TreeAdd(&t.root, ".rrrrrrrrrrrrrrrr", n18);
    Forest_TreeAdd(&t.root, ".rrrrrrrrrrrrrrrl", l17);
    Forest_TreeAdd(&t.root, ".rrrrrrrrrrrrrrrrr", n19);
    Forest_TreeAdd(&t.root, ".rrrrrrrrrrrrrrrrl", l18);
    Forest_TreeAdd(&t.root, ".rrrrrrrrrrrrrrrrrr", l19);
    Forest_TreeAdd(&t.root, ".rrrrrrrrrrrrrrrrrl", l20);

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

    char path2 = 0;
    char *s2 = NULL;
    int len2 = Forest_TreeSerialize(&s2, dst.root, &path2, 0, 0);

    if (len == len2) {
        for (int i=0; i<len; i++) {
            if (*(s+i) != *(s2+i)) {
                LG_DEBUG("FAILED: TREES NOT EQUAL.  Len = %d", len);
                return FOREST_ERR;
            }
        }
        LG_DEBUG("SUCCESS");
        return FOREST_OK;
    } else {
        LG_DEBUG("FAILED: TREE LENGTHS NOT EQUAL.  Lens = %d, %d", len, len2);
        return FOREST_ERR;
    }

}

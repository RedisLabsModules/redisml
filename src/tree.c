#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "reader.h"
#include "tree.h"
#include "util/logging.h"
#include "rmalloc.h"

Node *newNode(char *splitterAttr) {
  Node *n = (Node *)malloc(sizeof(Node));
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

Node *NewNumericalNode(char *splitterAttr, double sv) {
  Node *n = newNode(splitterAttr);
  n->splitterType = S_NUMERICAL;
  n->splitterVal.fval = sv;
  return n;
}

Node *NewCategoricalNode(char *splitterAttr, char *sv) {
  Node *n = newNode(splitterAttr);
  n->splitterType = S_CATEGORICAL;
  n->splitterVal.strval = malloc(strlen(sv) + 1);
  sprintf(n->splitterVal.strval, "%s", sv);
  return n;
}

Node *NewLeaf(double predVal) {
  Node *n = newNode(NULL);
  n->type = N_LEAF;
  n->predVal = predVal;
  return n;
}

int TreeAdd(Node **root, char *path, Node *n) {
  if (strlen(path) == 1) {
    *root = n;
    return TREE_OK;
  }

  if (*root == NULL) {
    printf("add node NULL ERR\n");
    return TREE_ERR;
  }
  if (*(path + 1) == 'r') {
    return TreeAdd(&(*root)->right, path + 1, n);
  }
  if (*(path + 1) == 'l') {
    return TreeAdd(&(*root)->left, path + 1, n);
  }

  return TREE_ERR;
}

Node *TreeGet(Node *root, char *path) {
  if (root == NULL) {
    return NULL;
  }

  if (strlen(path) == 1) {
    return root;
  }

  if (*(path + 1) == 'r') {
    return TreeGet(root->right, path + 1);
  }
  if (*(path + 1) == 'l') {
    return TreeGet(root->left, path + 1);
  }

  return NULL;
}

static void printNode(Node *n) {
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

void TreePrint(Node *root, char *path, int step) {
  if (root != NULL) {
    printf("%d ==> %s:", step, path);
    printNode(root);
    char *nextpath = malloc(strlen(path) + 2);
    sprintf(nextpath, "%sl", path);
    TreePrint(root->left, nextpath, step + 1);
    sprintf(nextpath, "%sr", path);
    TreePrint(root->right, nextpath, step + 1);
  }
}

int TreeSerialize(char **dst, Node *root, char *path, int plen, int slen) {
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
    splitteAttrLen = strlen(root->splitterAttr) + 1 + sizeof(SplitterType);
  }

  len += sizeof(int) + pathSize + 1 + sizeof(double) + splitteAttrLen +
         splitterSize;
  printf("len: %d\n", len);
  *dst = realloc(*dst, len);
  char *s = *dst;
  printf("s: %p\n", s);
  int pos = slen;

  printf("plen: %d\n", plen);
  *((int *)(s + pos)) = plen;
  printf("s:plen: %d\n", *((int *)(s + pos)));
  pos += sizeof(int);

  if (plen) {
    *(s + pos) = *path;
    pos += pathSize;
  }

  *(s + pos) = (char)root->type;
  pos++;

  *((double *)(s + pos)) = root->predVal;
  pos += sizeof(double);

  printf("pos: %d, len: %d\n", pos, len);
  if (root->type != N_LEAF) {
    strcpy(s + pos, root->splitterAttr);
    pos += strlen(root->splitterAttr) + 1;

    *((SplitterType *)(s + pos)) = root->splitterType;
    pos += sizeof(SplitterType);

    if (root->splitterType == S_NUMERICAL) {
      *((double *)(s + pos)) = root->splitterVal.fval;
      pos += sizeof(double);
    } else if (root->splitterType == S_CATEGORICAL) {
      strcpy(s + pos, root->splitterVal.strval);
      pos += strlen(root->splitterVal.strval) + 1;
    }

    printf("pos: %d, len: %d\n", pos, len);
    char nextpath = *path << 1;
    len = TreeSerialize(dst, root->left, &nextpath, plen + 1, len);
    nextpath |= 1;
    len = TreeSerialize(dst, root->right, &nextpath, plen + 1, len);
  }
  return len;
}

void TreeDeSerialize(char *s, Node **root, int slen) {
  int pos = 0;
  while (pos < slen) {
    int len = *((int *)(s + pos));
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

    NodeType type = (NodeType) * (s + pos);
    printf("nodetype: %d\n", type);
    pos++;

    double predVal = *((double *)(s + pos));
    printf("predVal: %lf\n", predVal);
    pos += sizeof(double);

    Node *n = NULL;
    if (type != N_LEAF) {
      char *splitterAttr = s + pos;
      pos += strlen(splitterAttr) + 1;

      SplitterType splitterType = (SplitterType) * (s + pos);
      pos += sizeof(SplitterType);

      if (splitterType == S_NUMERICAL) {
        //*((double *)s + pos) = root->splitterVal.fval;
        printf("numerical: %lf\n", *((double *)(s + pos)));
        n = NewNumericalNode(splitterAttr, *((double *)(s + pos)));
        pos += sizeof(double);
      } else if (splitterType == S_CATEGORICAL) {
        printf("categorical: %s\n", s + pos);
        n = NewCategoricalNode(splitterAttr, s + pos);
        // strcpy(s + pos, root->splitterVal.strval);
      }
    } else {
      n = NewLeaf(predVal);
    }
    TreeAdd(root, path, n);
    printf("last pos: %d\n", pos);
    // pos += 16;
  }
}

void TreeDel(Node *root) {
  if (root != NULL) {
    TreeDel(root->left);
    TreeDel(root->right);
    root->left = NULL;
    root->right = NULL;
    free(root);
  }
}

double TreeClassify(InputRow *ir, Node *root) {
  if (root == NULL) {
    printf("TreeClassify reached NULL node\n");
    return 0;
  }
  if (root->type == N_LEAF) {
    return root->predVal;
  }

  double attrVal = GetValue(ir, root->splitterAttr);
  if (attrVal <= root->splitterVal.fval) {
    return TreeClassify(ir, root->left);
  }
  return TreeClassify(ir, root->right);
}

void TreeTest() {
  // Field *irFields = NULL;
  // InputRow testIR = {0, irFields};
  // MakeInputRow("1:0.2,3:4,5:6,7:8", &testIR);
  // return;
  Tree t = {.root = NULL};
  // Node *n0 = NewNumericalNode("0", 0.0);
  Node *n1 = NewNumericalNode("1", 1.0);
  Node *n2 = NewNumericalNode("2", 2.0);
  Node *n3 = NewNumericalNode("3", 3.0);
  Node *n4 = NewNumericalNode("4", 4.0);

  Node *n5 = NewLeaf(5.0);
  Node *n6 = NewLeaf(6.0);
  Node *n7 = NewLeaf(7.0);
  Node *n8 = NewLeaf(8.0);
  Node *n9 = NewLeaf(9.0);

  // TreeGet(t.root, ".rr");

  // TreeAdd(&t.root, ".r", n);
  TreeAdd(&t.root, ".", n1);
  TreeAdd(&t.root, ".l", n2);
  TreeAdd(&t.root, ".r", n3);
  TreeAdd(&t.root, ".rl", n8);
  TreeAdd(&t.root, ".rr", n9);
  TreeAdd(&t.root, ".ll", n4);
  TreeAdd(&t.root, ".lr", n7);
  TreeAdd(&t.root, ".lll", n5);
  TreeAdd(&t.root, ".llr", n6);

  printf("1***1\n");
  TreePrint(t.root, "+", 0);
  printf("2***2\n");
  char path = 0;
  char *s = NULL;
  printf("s: %p\n", s);
  int len = TreeSerialize(&s, t.root, &path, 0, 0);
  printf("slen: %d\n", len);
  printf("sizeof(Node): %ld\n", sizeof(Node));

  Tree dst = {.root = NULL};
  printf("s: %p\n", s);
  TreeDeSerialize(s, &dst.root, len);
  printf("3***3\n");
  TreePrint(dst.root, "+", 0);
  printf("Original:\n");
  TreePrint(t.root, "+", 0);

  return;

  // Field
  // nputRow ir = {0, fields};
  // double res = TreeClassify(&ir, t.root);
  // printf("res=%f\n", res);

  // reeDel(t.root);
  // printf("4***4\n");
  // TreePrint(t.root, "*");
}

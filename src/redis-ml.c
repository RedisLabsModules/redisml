#include <stdio.h>
#include <time.h>

#include "redismodule.h"
#include "rmutil/strings.h"
#include "rmutil/test_util.h"
#include "rmutil/util.h"
#include "util/logging.h"
#include "reader.h"
#include "tree.h"
#include "reg.h"
#include "matrix.h"
#include "rmalloc.h"

#define FORESTTYPE_ENCODING_VERSION 0
#define FORESTTYPE_NAME "FOREST"
#define RLMODULE_NAME "REDIS-ML"
#define RLMODULE_VERSION "1.0.0"

#define LINREGTYPE_ENCODING_VERSION 0
#define LINREGTYPE_NAME "LINREG"

#define MATRIXTYPE_ENCODING_VERSION 0
#define MATRIXTYPE_NAME "MATRIX"

#define REDIS_ML_ERROR_GENERIC "ERR Generic"

static RedisModuleType *ForestType;
static RedisModuleType *LinRegType;
static RedisModuleType *MatrixType;

void *ForestTypeRdbLoad(RedisModuleIO *io, int encver) {
  if (encver != FORESTTYPE_ENCODING_VERSION) {
    return NULL;
  }
  Forest *f;
  f = malloc(sizeof(*f));
  f->len = RedisModule_LoadUnsigned(io);
  f->Trees = malloc(f->len * sizeof(Tree *));

  for (int i = 0; i < f->len; i++) {
    Tree *t = malloc(sizeof(Tree));
    f->Trees[i] = t;
    size_t tlen;
    char *s = RedisModule_LoadStringBuffer(io, &tlen);
    TreeDeSerialize(s, &t->root, tlen);
  }
  return f;
}

void ForestTypeRdbSave(RedisModuleIO *io, void *ptr) {
  Forest *f = ptr;
  RedisModule_SaveUnsigned(io, f->len);
  for (int i = 0; i < f->len; i++) {
    char path = 0;
    char *s = NULL;
    int len = TreeSerialize(&s, f->Trees[i]->root, &path, 0, 0);
    RedisModule_SaveStringBuffer(io, s, len);
    free(s);
  }
}

void ForestTypeAofRewrite(RedisModuleIO *aof, RedisModuleString *key,
                          void *value) {}

void ForestTypeDigest(RedisModuleDigest *digest, void *value) {}

void ForestTypeFree(void *value) {
  Forest *f = value;
  for (int i = 0; i < f->len; i++) {
    TreeDel(f->Trees[i]->root);
  }
  free(f);
}

int ForestTestCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  TreeTest();
  RedisModule_ReplyWithSimpleString(ctx, "TEST_OK");
  return REDISMODULE_OK;
}

// 2D array comperator for the qsort
static int cmp2D(const void *p1, const void *p2) {
  return (int)(((const double *)p2)[1] - ((const double *)p1)[1]);
}

// ml.forest.run <forest> <data_item> [CLASSIFICATION|REGRESSION]
int ForestRunCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  if (argc < 3) {
    return RedisModule_WrongArity(ctx);
  }

  RedisModule_AutoMemory(ctx);

  char *data;
  RMUtil_ParseArgs(argv, argc, 2, "c", &data);

  char *op = "CLASSIFICATION";
  if (argc == 4) {
    RMUtil_ParseArgs(argv, argc, 3, "c", &op);
  }
  int classification = strcmp(op, "CLASSIFICATION") == 0 ? 1 : 0;

  Forest *f;
  RedisModuleKey *key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ);
  int type = RedisModule_KeyType(key);
  if (type == REDISMODULE_KEYTYPE_EMPTY ||
      RedisModule_ModuleTypeGetType(key) != ForestType) {
    return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
  }
  f = RedisModule_ModuleTypeGetValue(key);

  Field *fields = NULL;
  InputRow ir = {0, fields};
  MakeInputRow(data, &ir);

  double rep = 0;
  if (classification) {
    LG_DEBUG("classification");
    double results[1024][2] = {{-1}};
    long class;
    long maxClass = 0;
    for (int i = 0; i < (int)f->len; i++) {
      class = (long)TreeClassify(&ir, f->Trees[i]->root) % 1024;
      results[class][0] = (double)class;
      results[class][1]++;
      if (class > maxClass) {
        maxClass = class;
      }
    }
    qsort(&results[0], 1024, sizeof(results[0]), cmp2D);
    rep = results[0][0];
  } else {
    LG_DEBUG("regression");
    for (int i = 0; i < (int)f->len; i++) {
      rep += TreeClassify(&ir, f->Trees[i]->root);
    }
    rep /= (int)f->len;
  }

  RedisModule_ReplyWithDouble(ctx, rep);
  return REDISMODULE_OK;
}

/*
 * rediforest.ADD <forestId> <treeId> <path> [[NUMERIC|CATEGORIC] <splitterAttr>
 * <splitterVal] | [LEAF] <predVal>]
 */
int ForestAddCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  if (argc < 6) {
    return RedisModule_WrongArity(ctx);
  }
  RedisModule_AutoMemory(ctx);

  RedisModuleString *fid;
  double tid;
  RMUtil_ParseArgs(argv, argc, 1, "sd", &fid, &tid);
  RedisModuleKey *key =
      RedisModule_OpenKey(ctx, fid, REDISMODULE_READ | REDISMODULE_WRITE);

  int type = RedisModule_KeyType(key);
  if (type != REDISMODULE_KEYTYPE_EMPTY &&
      RedisModule_ModuleTypeGetType(key) != ForestType) {
    return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
  }

  Forest *f;
  if (type == REDISMODULE_KEYTYPE_EMPTY) {
    f = malloc(sizeof(Forest));
    f->len = 0;
    f->Trees = NULL;
    RedisModule_ModuleTypeSetValue(key, ForestType, f);
  } else {
    f = RedisModule_ModuleTypeGetValue(key);
  }

  if (tid > (f->len)) {
    return RedisModule_ReplyWithError(ctx, REDIS_ML_ERROR_GENERIC);
  }

  Tree *t;
  if (tid == (f->len)) {
    f->len++;
    f->Trees = realloc(f->Trees, f->len * sizeof(Tree *));
    t = malloc(sizeof(Tree));
    f->Trees[f->len - 1] = t;
  } else {
    t = f->Trees[(int)tid];
  }

  Node *n = NULL;
  int argIdx = 3;
  while (argIdx < argc) {
    char *path;
    char *type;
    RMUtil_ParseArgs(argv, argc, argIdx, "cc", &path, &type);
    argIdx += 2;
    if (strncasecmp(type, "NUMERIC", strlen(type)) == 0) {
      char *splitterAttr;
      double splitterVal;
      RMUtil_ParseArgs(argv, argc, argIdx, "cd", &splitterAttr, &splitterVal);
      n = NewNumericalNode(splitterAttr, splitterVal);
      argIdx += 2;
    } else if (strncasecmp(type, "CATEGORIC", strlen(type)) == 0) {
      char *splitterAttr;
      char *splitterVal;
      RMUtil_ParseArgs(argv, argc, argIdx, "cc", &splitterAttr, &splitterVal);
      n = NewCategoricalNode(splitterAttr, splitterVal);
      argIdx += 2;
    } else if (strncasecmp(type, "LEAF", strlen(type)) == 0) {
      double predVal;
      RMUtil_ParseArgs(argv, argc, argIdx, "d", &predVal);
      n = NewLeaf(predVal);
      argIdx++;
    } else {
      return REDISMODULE_ERR;
    }
    int err = TreeAdd(&t->root, path, n);
    if (err != TREE_OK) {
      return REDISMODULE_ERR;
    }
  }
  RedisModule_ReplyWithSimpleString(ctx, "OK");
  return REDISMODULE_OK;
}

// LINREG Section:

void *LinRegTypeRdbLoad(RedisModuleIO *io, int encver) {
  if (encver != LINREGTYPE_ENCODING_VERSION) {
    return NULL;
  }
  return NULL;
}

void LinRegTypeRdbSave(RedisModuleIO *io, void *ptr) {}

void LinRegTypeAofRewrite(RedisModuleIO *aof, RedisModuleString *key,
                          void *value) {}

void LinRegTypeDigest(RedisModuleDigest *digest, void *value) {}

void LinRegTypeFree(void *value) {}

/*
 * Create / Override a linear regression model
 * ml.linreg.load <Id> <intercept> <coefficients ...>
 */
int LinRegSetCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  if (argc < 4) {
    return RedisModule_WrongArity(ctx);
  }
  RedisModule_AutoMemory(ctx);

  RedisModuleString *id;
  RMUtil_ParseArgs(argv, argc, 1, "s", &id);
  RedisModuleKey *key =
      RedisModule_OpenKey(ctx, id, REDISMODULE_READ | REDISMODULE_WRITE);

  int type = RedisModule_KeyType(key);
  if (type != REDISMODULE_KEYTYPE_EMPTY &&
      RedisModule_ModuleTypeGetType(key) != LinRegType) {
    return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
  }

  LinReg *lr;
  if (type == REDISMODULE_KEYTYPE_EMPTY) {
    lr = malloc(sizeof(LinReg));
    RedisModule_ModuleTypeSetValue(key, LinRegType, lr);
  } else {
    lr = RedisModule_ModuleTypeGetValue(key);
  }
  lr->clen = argc - 2;
  lr->coefficients = malloc(lr->clen * sizeof(double));
  RMUtil_ParseArgs(argv, argc, 2, "d", &lr->intercept);
  int argIdx = 3;
  while (argIdx < argc) {
    RMUtil_ParseArgs(argv, argc, argIdx, "d", &lr->coefficients[argIdx - 3]);
    argIdx++;
  }
  RedisModule_ReplyWithSimpleString(ctx, "OK");
  return REDISMODULE_OK;
}

/*Predict value for a set of features
* ml.linreg.predict <id> <features ...>
*/
int LinRegPredictCommand(RedisModuleCtx *ctx, RedisModuleString **argv,
                         int argc) {
  if (argc < 3) {
    return RedisModule_WrongArity(ctx);
  }
  RedisModule_AutoMemory(ctx);

  LinReg *lr;
  RedisModuleKey *key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ);
  int type = RedisModule_KeyType(key);
  if (type == REDISMODULE_KEYTYPE_EMPTY ||
      RedisModule_ModuleTypeGetType(key) != LinRegType) {
    return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
  }
  lr = RedisModule_ModuleTypeGetValue(key);

  double *features = malloc(lr->clen * sizeof(double));
  int argIdx = 2;
  while (argIdx < argc) {
    RMUtil_ParseArgs(argv, argc, argIdx, "d", &features[argIdx - 2]);
    argIdx++;
  }
  double rep = LinRegPredict(features, lr);
  RedisModule_ReplyWithDouble(ctx, rep);
  return REDISMODULE_OK;
}

/*Predict value for a set of features
* ml.logreg.predict <id> <features ...>
*/
int LogRegPredictCommand(RedisModuleCtx *ctx, RedisModuleString **argv,
                         int argc) {
  if (argc < 3) {
    return RedisModule_WrongArity(ctx);
  }
  RedisModule_AutoMemory(ctx);

  LinReg *lr;
  RedisModuleKey *key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ);
  int type = RedisModule_KeyType(key);
  if (type == REDISMODULE_KEYTYPE_EMPTY ||
      RedisModule_ModuleTypeGetType(key) != LinRegType) {
    return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
  }
  lr = RedisModule_ModuleTypeGetValue(key);

  double *features = malloc(lr->clen * sizeof(double));
  int argIdx = 2;
  while (argIdx < argc) {
    RMUtil_ParseArgs(argv, argc, argIdx, "d", &features[argIdx - 2]);
    argIdx++;
  }
  double rep = LogRegPredict(features, lr);
  RedisModule_ReplyWithDouble(ctx, rep);
  return REDISMODULE_OK;
}

// MATRIX Section:

void *MatrixTypeRdbLoad(RedisModuleIO *io, int encver) {
  if (encver != MATRIXTYPE_ENCODING_VERSION) {
    return NULL;
  }
  return NULL;
}

void MatrixTypeRdbSave(RedisModuleIO *io, void *ptr) {}

void MatrixTypeAofRewrite(RedisModuleIO *aof, RedisModuleString *key,
                          void *value) {}

void MatrixTypeDigest(RedisModuleDigest *digest, void *value) {}

void MatrixTypeFree(void *value) {}

/*
 * Create / Override a linear regression model
 * ml.linreg.load <Id> <intercept> <coefficients ...>
 */
int MatrixSetCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  if (argc < 4) {
    return RedisModule_WrongArity(ctx);
  }
  RedisModule_AutoMemory(ctx);

  RedisModuleKey *key =
      RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);

  int type = RedisModule_KeyType(key);
  if (type != REDISMODULE_KEYTYPE_EMPTY &&
      RedisModule_ModuleTypeGetType(key) != MatrixType) {
    return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
  }

  Matrix *m;
  if (type == REDISMODULE_KEYTYPE_EMPTY) {
    m = malloc(sizeof(Matrix));
    RedisModule_ModuleTypeSetValue(key, MatrixType, m);
  } else {
    m = RedisModule_ModuleTypeGetValue(key);
  }
  RMUtil_ParseArgs(argv, argc, 2, "ll", &(m->rows), &(m->cols));
  LG_DEBUG("rows: %lld, cols: %lld\n", m->rows, m->cols);
  m->values = malloc(m->cols * m->rows * sizeof(double));
  int argIdx = 4;
  while (argIdx < argc) {
    RMUtil_ParseArgs(argv, argc, argIdx, "d", &m->values[argIdx - 4]);
    argIdx++;
  }
  RedisModule_ReplyWithSimpleString(ctx, "OK");
  return REDISMODULE_OK;
}

/*Multiply matrices a*b=c. a and b are existng keys and c should be new /
* overwritten
* ml.matrix.multiply a b c
*/
int MatrixMultiplyCommand(RedisModuleCtx *ctx, RedisModuleString **argv,
                          int argc) {
  if (argc != 4) {
    return RedisModule_WrongArity(ctx);
  }
  RedisModule_AutoMemory(ctx);

  Matrix *a, *b, *c;
  RedisModuleKey *key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ);
  int type = RedisModule_KeyType(key);
  if (type == REDISMODULE_KEYTYPE_EMPTY ||
      RedisModule_ModuleTypeGetType(key) != MatrixType) {
    return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
  }
  a = RedisModule_ModuleTypeGetValue(key);

  key = RedisModule_OpenKey(ctx, argv[2], REDISMODULE_READ);
  type = RedisModule_KeyType(key);
  if (type == REDISMODULE_KEYTYPE_EMPTY ||
      RedisModule_ModuleTypeGetType(key) != MatrixType) {
    return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
  }
  b = RedisModule_ModuleTypeGetValue(key);

  key = RedisModule_OpenKey(ctx, argv[3], REDISMODULE_READ | REDISMODULE_WRITE);
  type = RedisModule_KeyType(key);
  if (type != REDISMODULE_KEYTYPE_EMPTY &&
      RedisModule_ModuleTypeGetType(key) != MatrixType) {
    return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
  }

  if (type == REDISMODULE_KEYTYPE_EMPTY) {
    c = malloc(sizeof(Matrix));
    RedisModule_ModuleTypeSetValue(key, MatrixType, c);
  } else {
    c = RedisModule_ModuleTypeGetValue(key);
  }
  c->rows = a->rows;
  c->cols = b->cols;
  c->values = malloc(c->cols * c->rows * sizeof(double));
  MatrixMultiply(*a, *b, *c);
  RedisModule_ReplyWithSimpleString(ctx, "OK");
  return REDISMODULE_OK;
}

/*Add matrices c=a+b. a and b are existng keys and c should be new /
* overwritten
* ml.matrix.add a b c
*/
int MatrixAddCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  if (argc != 4) {
    return RedisModule_WrongArity(ctx);
  }
  RedisModule_AutoMemory(ctx);

  Matrix *a, *b, *c;
  RedisModuleKey *key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ);
  int type = RedisModule_KeyType(key);
  if (type == REDISMODULE_KEYTYPE_EMPTY ||
      RedisModule_ModuleTypeGetType(key) != MatrixType) {
    return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
  }
  a = RedisModule_ModuleTypeGetValue(key);

  key = RedisModule_OpenKey(ctx, argv[2], REDISMODULE_READ);
  type = RedisModule_KeyType(key);
  if (type == REDISMODULE_KEYTYPE_EMPTY ||
      RedisModule_ModuleTypeGetType(key) != MatrixType) {
    return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
  }
  b = RedisModule_ModuleTypeGetValue(key);

  key = RedisModule_OpenKey(ctx, argv[3], REDISMODULE_READ | REDISMODULE_WRITE);
  type = RedisModule_KeyType(key);
  if (type != REDISMODULE_KEYTYPE_EMPTY &&
      RedisModule_ModuleTypeGetType(key) != MatrixType) {
    return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
  }

  if (type == REDISMODULE_KEYTYPE_EMPTY) {
    c = malloc(sizeof(Matrix));
    RedisModule_ModuleTypeSetValue(key, MatrixType, c);
  } else {
    c = RedisModule_ModuleTypeGetValue(key);
  }
  c->rows = a->rows;
  c->cols = b->cols;
  c->values = malloc(c->cols * c->rows * sizeof(double));
  MatrixAdd(*a, *b, *c);
  RedisModule_ReplyWithSimpleString(ctx, "OK");
  return REDISMODULE_OK;
}

/*Scale matrix m by scalar n
* ml.matrix.scale m n
*/
int MatrixScaleCommand(RedisModuleCtx *ctx, RedisModuleString **argv,
                       int argc) {
  if (argc != 3) {
    return RedisModule_WrongArity(ctx);
  }
  RedisModule_AutoMemory(ctx);

  Matrix *m;
  double n;
  RedisModuleKey *key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ);
  int type = RedisModule_KeyType(key);
  if (type == REDISMODULE_KEYTYPE_EMPTY ||
      RedisModule_ModuleTypeGetType(key) != MatrixType) {
    return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
  }
  m = RedisModule_ModuleTypeGetValue(key);
  RMUtil_ParseArgs(argv, argc, 2, "d", &n);
  MatrixScale(*m, n);
  RedisModule_ReplyWithSimpleString(ctx, "OK");
  return REDISMODULE_OK;
}

/*
* Get a matrix
*/
int MatrixGetCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  if (argc != 2) {
    return RedisModule_WrongArity(ctx);
  }
  RedisModule_AutoMemory(ctx);

  Matrix *m;
  RedisModuleKey *key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ);
  int type = RedisModule_KeyType(key);
  if (type == REDISMODULE_KEYTYPE_EMPTY ||
      RedisModule_ModuleTypeGetType(key) != MatrixType) {
    return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
  }
  m = RedisModule_ModuleTypeGetValue(key);
  RedisModule_ReplyWithArray(ctx, m->rows * m->cols + 2);
  RedisModule_ReplyWithLongLong(ctx, m->rows);
  RedisModule_ReplyWithLongLong(ctx, m->cols);
  for (int i = 0; i < m->rows * m->cols; i++) {
    RedisModule_ReplyWithDouble(ctx, m->values[i]);
  }
  return REDISMODULE_OK;
}

int MatrixPrintCommand(RedisModuleCtx *ctx, RedisModuleString **argv,
                       int argc) {
  if (argc != 2) {
    return RedisModule_WrongArity(ctx);
  }
  RedisModule_AutoMemory(ctx);

  Matrix *m;
  RedisModuleKey *key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ);
  int type = RedisModule_KeyType(key);
  if (type == REDISMODULE_KEYTYPE_EMPTY ||
      RedisModule_ModuleTypeGetType(key) != MatrixType) {
    return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
  }
  m = RedisModule_ModuleTypeGetValue(key);

  MatrixPrint(*m);
  RedisModule_ReplyWithSimpleString(ctx, "OK");
  return REDISMODULE_OK;
}

int MatrixTestCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  MatrixTest();
  RedisModule_ReplyWithSimpleString(ctx, "OK");
  return REDISMODULE_OK;
}

// Initialize the module
int RedisModule_OnLoad(RedisModuleCtx *ctx) {
  // Register the module itself
  if (RedisModule_Init(ctx, "redis-ml", 1, REDISMODULE_APIVER_1) ==
      REDISMODULE_ERR) {
    return REDISMODULE_ERR;
  }

  // Register Forest data type and functions
  ForestType = RedisModule_CreateDataType(
      ctx, FORESTTYPE_NAME, FORESTTYPE_ENCODING_VERSION, ForestTypeRdbLoad,
      ForestTypeRdbSave, ForestTypeAofRewrite, ForestTypeDigest,
      ForestTypeFree);

  RMUtil_RegisterWriteCmd(ctx, "ml.forest.add", ForestAddCommand);
  RMUtil_RegisterWriteCmd(ctx, "ml.forest.run", ForestRunCommand);
  RMUtil_RegisterWriteCmd(ctx, "ml.forest.test", ForestTestCommand);

  // Register LINREG data type and functions
  LinRegType = RedisModule_CreateDataType(
      ctx, LINREGTYPE_NAME, LINREGTYPE_ENCODING_VERSION, LinRegTypeRdbLoad,
      LinRegTypeRdbSave, LinRegTypeAofRewrite, LinRegTypeDigest,
      LinRegTypeFree);

  RMUtil_RegisterWriteCmd(ctx, "ml.linreg.set", LinRegSetCommand);
  RMUtil_RegisterWriteCmd(ctx, "ml.linreg.predict", LinRegPredictCommand);
  RMUtil_RegisterWriteCmd(ctx, "ml.logreg.set", LinRegSetCommand);
  RMUtil_RegisterWriteCmd(ctx, "ml.logreg.predict", LogRegPredictCommand);

  // Register MATRIX data type and functions
  MatrixType = RedisModule_CreateDataType(
      ctx, MATRIXTYPE_NAME, MATRIXTYPE_ENCODING_VERSION, MatrixTypeRdbLoad,
      MatrixTypeRdbSave, MatrixTypeAofRewrite, MatrixTypeDigest,
      MatrixTypeFree);

  RMUtil_RegisterWriteCmd(ctx, "ml.matrix.set", MatrixSetCommand);
  RMUtil_RegisterWriteCmd(ctx, "ml.matrix.get", MatrixGetCommand);
  RMUtil_RegisterWriteCmd(ctx, "ml.matrix.multiply", MatrixMultiplyCommand);
  RMUtil_RegisterWriteCmd(ctx, "ml.matrix.add", MatrixAddCommand);
  RMUtil_RegisterWriteCmd(ctx, "ml.matrix.scale", MatrixScaleCommand);
  RMUtil_RegisterWriteCmd(ctx, "ml.matrix.print", MatrixPrintCommand);
  RMUtil_RegisterWriteCmd(ctx, "ml.matrix.test", MatrixTestCommand);

  LOGGING_INIT(L_WARN);
  LG_DEBUG("module loaded ok.");
  return REDISMODULE_OK;
}

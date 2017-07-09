#include <stdio.h>
#include <ctype.h>

#include "redismodule.h"
#include "rmutil/test_util.h"
#include "util/logging.h"
#include "feature-vec.h"
#include "forest.h"
#include "reg.h"
#include "matrix-type.h"
#include "forest-type.h"
#include "regression-type.h"
#include "util/thpool.h"

#define RLMODULE_NAME "REDIS-ML"
#define RLMODULE_VERSION "1.0.0"

#define REDIS_ML_ERROR_GENERIC "ERR Generic"

/*================ Forest Commands ================*/

int ForestTestCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    Forest_TreeTest();
    RedisModule_ReplyWithSimpleString(ctx, "TEST_OK");
    return REDISMODULE_OK;
}

/*ml.forest.run <forest> <data_item> [CLASSIFICATION|REGRESSION]*/
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

    Feature *features = NULL;
    FeatureVec fv = {0, features};
    if (FeatureVec_Create(data, &fv) != 0) {
        return RedisModule_ReplyWithError(ctx, REDIS_ML_FV_ERROR_BAD_FORMAT);
    }

    double rep = Forest_Classify(fv, f, classification);
    RedisModule_ReplyWithDouble(ctx, rep);
    return REDISMODULE_OK;
}

/*ml.forest.print <forest> <tree_id>*/
int ForestPrintCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (argc < 3) {
        return RedisModule_WrongArity(ctx);
    }

    RedisModule_AutoMemory(ctx);

    char *tid;
    RMUtil_ParseArgs(argv, argc, 2, "c", &tid);

    Forest *f;
    RedisModuleKey *key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ);
    int type = RedisModule_KeyType(key);
    if (type == REDISMODULE_KEYTYPE_EMPTY ||
        RedisModule_ModuleTypeGetType(key) != ForestType) {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }
    f = RedisModule_ModuleTypeGetValue(key);

    Forest_TreePrint(f->Trees[atoi(tid)]->root, ".", 0);
    RedisModule_ReplyWithSimpleString(ctx, "todo: return as string\n");
    return REDISMODULE_OK;
}

/*
 * rediforest.ADD <forestId> <treeId> <path> [[NUMERIC|CATEGORIC] <splitterAttr>
 * <splitterVal] | [LEAF] <predVal> <stats>]
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

    int argIdx = 3;
    char *path;
    char *nodeType;
    char *hasStats;

    /* Check args */
    while (argIdx < argc) {
        RMUtil_ParseArgs(argv, argc, argIdx, "cc", &path, &nodeType);
        argIdx += 2;

        /* Check path and convert to lower case */
        for (int i = 1; i < strlen(path); ++i) {
            path[i] = tolower(path[i]);
            if (!(path[i] == 'l' || path[i] == 'r')) {
                return RedisModule_ReplyWithError(ctx, REDIS_ML_FOREST_ERROR_WRONG_PATH);
            }
        }

        /* Check node type */
        if (strncasecmp(nodeType, "NUMERIC", strlen(nodeType)) == 0) {
            argIdx += 2;
            continue;
        } else if (strncasecmp(nodeType, "CATEGORIC", strlen(nodeType)) == 0) {
            argIdx += 2;
            continue;
        } else if (strncasecmp(nodeType, "LEAF", strlen(nodeType)) == 0) {
            argIdx++;
            RMUtil_ParseArgs(argv, argc, argIdx, "c", &hasStats);
            if (strncasecmp(hasStats, "STATS", strlen(hasStats)) == 0) {
                argIdx+=2;
            }
            continue;
        } else {
            return RedisModule_ReplyWithError(ctx, REDIS_ML_FOREST_ERROR_WRONG_SPLIT_TYPE);
        }
    }
    LG_DEBUG("params ok");

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

    Forest_Tree *t;
    if (tid == (f->len)) {
        f->len++;
        f->Trees = realloc(f->Trees, f->len * sizeof(Forest_Tree *));
        t = malloc(sizeof(Forest_Tree));
        f->Trees[f->len - 1] = t;
    } else {
        t = f->Trees[(int) tid];
    }

    __forest_Node *n = NULL;
    argIdx = 3;
    while (argIdx < argc) {
        RMUtil_ParseArgs(argv, argc, argIdx, "cc", &path, &nodeType);
        argIdx += 2;
        if (strncasecmp(nodeType, "NUMERIC", strlen(nodeType)) == 0) {
            char *splitterAttr;
            double splitterVal;
            RMUtil_ParseArgs(argv, argc, argIdx, "cd", &splitterAttr, &splitterVal);
            n = Forest_NewNumericalNode(splitterAttr, splitterVal);
            argIdx += 2;
        } else if (strncasecmp(nodeType, "CATEGORIC", strlen(nodeType)) == 0) {
            char *splitterAttr;
            char *splitterVal;
            RMUtil_ParseArgs(argv, argc, argIdx, "cc", &splitterAttr, &splitterVal);
            n = Forest_NewCategoricalNode(splitterAttr, splitterVal);
            argIdx += 2;
        } else if (strncasecmp(nodeType, "LEAF", strlen(nodeType)) == 0) {
            double predVal;
            RMUtil_ParseArgs(argv, argc, argIdx, "d", &predVal);
            argIdx++;
            char *stats = NULL;
            RMUtil_ParseArgs(argv, argc, argIdx, "c", &hasStats);
            if (strncasecmp(hasStats, "STATS", strlen(hasStats)) == 0) {
                argIdx++;
                RMUtil_ParseArgs(argv, argc, argIdx, "c", &stats);
                argIdx++;
            }
            n = Forest_NewLeaf(predVal, stats);
        } else {
            return RedisModule_ReplyWithError(ctx, REDIS_ML_FOREST_ERROR_WRONG_SPLIT_TYPE);
        }
        int err = Forest_TreeAdd(&t->root, path, n);
        if (err != FOREST_OK) {
            return REDISMODULE_ERR;
        }
    }
    /*Check tree has no nulls*/
    if (Forest_CheckTree(t) != FOREST_OK) {
        RedisModule_DeleteKey(key);
        return RedisModule_ReplyWithError(ctx, REDIS_ML_FOREST_ERROR_NULL_NODES);
        return REDISMODULE_ERR;
    }
    /*Normalize tree values*/
    Forest_NormalizeTree(t);
#ifdef FOREST_USE_FAST_TREE
    Forest_GenFastTree(t);
#endif
    RedisModule_ReplyWithSimpleString(ctx, "OK");
    return REDISMODULE_OK;
}

/*================ Linear regression commands ================*/

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
        RedisModule_ModuleTypeGetType(key) != RegressionType) {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }

    LinReg *lr = NULL;
    if (type == REDISMODULE_KEYTYPE_EMPTY) {
        lr = malloc(sizeof(LinReg));
        lr->coefficients = NULL;
        RedisModule_ModuleTypeSetValue(key, RegressionType, lr);
    } else {
        lr = RedisModule_ModuleTypeGetValue(key);
    }
    lr->clen = argc - 2;
    lr->coefficients = realloc(lr->coefficients, lr->clen * sizeof(double));
    RMUtil_ParseArgs(argv, argc, 2, "d", &lr->intercept);
    int argIdx = 3;
    while (argIdx < argc) {
        RMUtil_ParseArgs(argv, argc, argIdx, "d", &lr->coefficients[argIdx - 3]);
        argIdx++;
    }
    RedisModule_ReplyWithSimpleString(ctx, "OK");
    return REDISMODULE_OK;
}

/*Run linear regression for a feature vector
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
        RedisModule_ModuleTypeGetType(key) != RegressionType) {
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

/*================ Logistic regression commands ================*/

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
        RedisModule_ModuleTypeGetType(key) != RegressionType) {
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

/*================ MATRIX Commands ================*/

/*
 * Create / Override a matrix
 * ml.matrix.set <Id> <rows> <cols> <values ...>
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
        m->values = NULL;
        RedisModule_ModuleTypeSetValue(key, MatrixType, m);
    } else {
        m = RedisModule_ModuleTypeGetValue(key);
    }
    RMUtil_ParseArgs(argv, argc, 2, "ll", &(m->rows), &(m->cols));
    LG_DEBUG("rows: %lld, cols: %lld\n", m->rows, m->cols);
    if (m->rows <= 0 || m->cols <= 0) {
        return RedisModule_ReplyWithError(ctx, REDIS_ML_MATRIX_SET_BAD_DIMENTIONS);
    }
    m->values = realloc(m->values, m->cols * m->rows * sizeof(double));
    int argIdx = 4;
    while (argIdx < argc) {
        RMUtil_ParseArgs(argv, argc, argIdx, "d", &m->values[argIdx - 4]);
        argIdx++;
    }
    RedisModule_ReplyWithSimpleString(ctx, "OK");
    return REDISMODULE_OK;
}

/*
 * Update a value in a matrix
 * ml.matrix.update <Id> <row> <col> <value>
 */
int MatrixUpdateCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (argc < 4) {
        return RedisModule_WrongArity(ctx);
    }
    RedisModule_AutoMemory(ctx);

    RedisModuleKey *key =
            RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);

    int type = RedisModule_KeyType(key);
    if (type == REDISMODULE_KEYTYPE_EMPTY || RedisModule_ModuleTypeGetType(key) != MatrixType) {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }

    Matrix *m;
    m = RedisModule_ModuleTypeGetValue(key);
    
    int row = 0;
    int col =0;
    double val = 0;
    RMUtil_ParseArgs(argv, argc, 2, "ll", &row, &col);
    if (row > m->rows || col > m->cols) {
        return RedisModule_ReplyWithError(ctx, REDIS_ML_MATRIX_SET_BAD_DIMENTIONS);
    }
    RMUtil_ParseArgs(argv, argc, 4, "d", &val);
    m->values[row * col] = val;
    RedisModule_ReplyWithSimpleString(ctx, "OK");
    return REDISMODULE_OK;
}

/*Multiply matrices a*b=c. a and b are existing keys and c should be new or
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

    if (b->rows != a->cols) {
        return RedisModule_ReplyWithError(ctx, REDIS_ML_MATRIX_MUL_BAD_DIMENTIONS);
    }

    key = RedisModule_OpenKey(ctx, argv[3], REDISMODULE_READ | REDISMODULE_WRITE);
    type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_EMPTY &&
        RedisModule_ModuleTypeGetType(key) != MatrixType) {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }

    if (type == REDISMODULE_KEYTYPE_EMPTY) {
        c = malloc(sizeof(Matrix));
        c->values = NULL;
        RedisModule_ModuleTypeSetValue(key, MatrixType, c);
    } else {
        c = RedisModule_ModuleTypeGetValue(key);
    }
    c->rows = a->rows;
    c->cols = b->cols;
    c->values = realloc(c->values, c->cols * c->rows * sizeof(double));
    Matrix_Multiply(*a, *b, *c);
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

    if (b->rows != a->rows || b->cols != a->cols) {
        return RedisModule_ReplyWithError(ctx, REDIS_ML_MATRIX_ADD_BAD_DIMENTIONS);
    }

    key = RedisModule_OpenKey(ctx, argv[3], REDISMODULE_READ | REDISMODULE_WRITE);
    type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_EMPTY &&
        RedisModule_ModuleTypeGetType(key) != MatrixType) {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }

    if (type == REDISMODULE_KEYTYPE_EMPTY) {
        c = malloc(sizeof(Matrix));
        c->values = NULL;
        RedisModule_ModuleTypeSetValue(key, MatrixType, c);
    } else {
        c = RedisModule_ModuleTypeGetValue(key);
    }
    c->rows = a->rows;
    c->cols = b->cols;
    c->values = realloc(c->values, c->cols * c->rows * sizeof(double));
    Matrix_Add(*a, *b, *c);
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
    Matrix_Scale(*m, n);
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

    Matrix_Print(*m);
    RedisModule_ReplyWithSimpleString(ctx, "OK");
    return REDISMODULE_OK;
}

int MatrixTestCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    Matrix_Test();
    RedisModule_ReplyWithSimpleString(ctx, "OK");
    return REDISMODULE_OK;
}

/*Initialize the module*/
int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
//int RedisModule_OnLoad(RedisModuleCtx *ctx) {
    /* Register the module itself*/
    if (RedisModule_Init(ctx, "redis-ml", 1, REDISMODULE_APIVER_1) ==
        REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    if (argc == 1) {
        RMUtil_ParseArgs(argv, argc, 0, "l", &FOREST_NUM_THREADS);
    } else {
        FOREST_NUM_THREADS = 1;
    }

    /* Register Forest data type and functions*/
    if (ForestTypeRegister(ctx) == REDISMODULE_ERR) return REDISMODULE_ERR;

    RMUtil_RegisterWriteCmd(ctx, "ml.forest.add", ForestAddCommand);
    RMUtil_RegisterReadCmd(ctx, "ml.forest.run", ForestRunCommand);
    RMUtil_RegisterWriteCmd(ctx, "ml.forest.test", ForestTestCommand);
    RMUtil_RegisterWriteCmd(ctx, "ml.forest.print", ForestPrintCommand);

    /*Register LINREG data type and commands*/
    if (RegressionTypeRegister(ctx) == REDISMODULE_ERR) return REDISMODULE_ERR;

    RMUtil_RegisterWriteCmd(ctx, "ml.linreg.set", LinRegSetCommand);
    RMUtil_RegisterReadCmd(ctx, "ml.linreg.predict", LinRegPredictCommand);
    RMUtil_RegisterWriteCmd(ctx, "ml.logreg.set", LinRegSetCommand);
    RMUtil_RegisterReadCmd(ctx, "ml.logreg.predict", LogRegPredictCommand);

    /*Register MATRIX data type and commands*/
    if (MatrixTypeRegister(ctx) == REDISMODULE_ERR) return REDISMODULE_ERR;

    RMUtil_RegisterWriteCmd(ctx, "ml.matrix.set", MatrixSetCommand);
    RMUtil_RegisterReadCmd(ctx, "ml.matrix.get", MatrixGetCommand);
    RMUtil_RegisterWriteCmd(ctx, "ml.matrix.multiply", MatrixMultiplyCommand);
    RMUtil_RegisterWriteCmd(ctx, "ml.matrix.add", MatrixAddCommand);
    RMUtil_RegisterWriteCmd(ctx, "ml.matrix.scale", MatrixScaleCommand);
    RMUtil_RegisterReadCmd(ctx, "ml.matrix.print", MatrixPrintCommand);
    RMUtil_RegisterWriteCmd(ctx, "ml.matrix.test", MatrixTestCommand);

    LOGGING_INIT(L_ERROR);

    Forest_thpool = thpool_init(FOREST_NUM_THREADS);

    printf("module loaded ok, threads: %d\n", FOREST_NUM_THREADS);
    return REDISMODULE_OK;
}

#include <stdio.h>

#include "redismodule.h"
#include "rmutil/test_util.h"
#include "util/logging.h"
#include "feature-vec.h"
#include "forest.h"
#include "reg.h"
#include "matrix-type.h"
#include "forest-type.h"
#include "regression-type.h"

#define RLMODULE_NAME "REDIS-ML"
#define RLMODULE_VERSION "1.0.0"

#define REDIS_ML_ERROR_GENERIC "ERR Generic"

/*================ Forest Commands ================*/

int ForestTestCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    Forest_TreeTest();
    RedisModule_ReplyWithSimpleString(ctx, "TEST_OK");
    return REDISMODULE_OK;
}

/*2D array comperator for the qsort*/
static int cmp2D(const void *p1, const void *p2) {
    return (int) (((const double *) p2)[1] - ((const double *) p1)[1]);
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
    MakeFeatureVec(data, &fv);

    double rep = 0;
    if (classification) {
        LG_DEBUG("classification");
        double results[1024][2] = {{-1}};
        long class;
        long maxClass = 0;
        for (int i = 0; i < (int) f->len; i++) {
            class = (long) Forest_TreeClassify(&fv, f->Trees[i]->root) % 1024;
            results[class][0] = (double) class;
            results[class][1]++;
            if (class > maxClass) {
                maxClass = class;
            }
        }
        qsort(&results[0], 1024, sizeof(results[0]), cmp2D);
        rep = results[0][0];
    } else {
        LG_DEBUG("regression");
        for (int i = 0; i < (int) f->len; i++) {
            rep += Forest_TreeClassify(&fv, f->Trees[i]->root);
        }
        rep /= (int) f->len;
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
        t = f->Trees[(int) tid];
    }

    __forest_Node *n = NULL;
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
            n = Forest_NewNumericalNode(splitterAttr, splitterVal);
            argIdx += 2;
        } else if (strncasecmp(type, "CATEGORIC", strlen(type)) == 0) {
            char *splitterAttr;
            char *splitterVal;
            RMUtil_ParseArgs(argv, argc, argIdx, "cc", &splitterAttr, &splitterVal);
            n = Forest_NewCategoricalNode(splitterAttr, splitterVal);
            argIdx += 2;
        } else if (strncasecmp(type, "LEAF", strlen(type)) == 0) {
            double predVal;
            RMUtil_ParseArgs(argv, argc, argIdx, "d", &predVal);
            n = Forest_NewLeaf(predVal);
            argIdx++;
        } else {
            return REDISMODULE_ERR;
        }
        int err = Forest_TreeAdd(&t->root, path, n);
        if (err != FOREST_OK) {
            return REDISMODULE_ERR;
        }
    }
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

    LinReg *lr;
    if (type == REDISMODULE_KEYTYPE_EMPTY) {
        lr = malloc(sizeof(LinReg));
        RedisModule_ModuleTypeSetValue(key, RegressionType, lr);
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
        RedisModule_ModuleTypeSetValue(key, MatrixType, m);
    } else {
        m = RedisModule_ModuleTypeGetValue(key);
    }
    RMUtil_ParseArgs(argv, argc, 2, "ll", &(m->rows), &(m->cols));
    LG_DEBUG("rows: %lld, cols: %lld\n", m->rows, m->cols);
    if (m->rows <= 0 || m->cols <= 0) {
        return RedisModule_ReplyWithError(ctx, REDIS_ML_ERROR_GENERIC);
    }
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

    if (b->rows != a->cols) {
        return RedisModule_ReplyWithError(ctx, REDIS_ML_ERROR_GENERIC);
    }

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

/*Initialize the module*/
int RedisModule_OnLoad(RedisModuleCtx *ctx) {
    /* Register the module itself*/
    if (RedisModule_Init(ctx, "redis-ml", 1, REDISMODULE_APIVER_1) ==
        REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    /* Register Forest data type and functions*/
    if (ForestTypeRegister(ctx) == REDISMODULE_ERR) return REDISMODULE_ERR;

    RMUtil_RegisterWriteCmd(ctx, "ml.forest.add", ForestAddCommand);
    RMUtil_RegisterReadCmd(ctx, "ml.forest.run", ForestRunCommand);
    RMUtil_RegisterWriteCmd(ctx, "ml.forest.test", ForestTestCommand);

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

    LOGGING_INIT(L_WARN);
    LG_DEBUG("module loaded ok.");
    return REDISMODULE_OK;
}

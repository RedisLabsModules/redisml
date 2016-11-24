#ifndef __MATRIX_TYPE_H__
#define __MATRIX_TYPE_H__

#include "redismodule.h"
#include "matrix.h"

#define MATRIXTYPE_ENCODING_VERSION 0
#define MATRIXTYPE_NAME "RLML_MTRX"

#define REDIS_ML_MATRIX_SET_BAD_DIMENTIONS "Matrix dimentions should be positive"
#define REDIS_ML_MATRIX_MUL_BAD_DIMENTIONS "Wrong matrix dimentions b:rows should equal a:cols"
#define REDIS_ML_MATRIX_ADD_BAD_DIMENTIONS "Added matrices dimentions should be equal"

extern RedisModuleType *MatrixType;

void *MatrixTypeRdbLoad(RedisModuleIO *, int);

void MatrixTypeRdbSave(RedisModuleIO *, void *);

void MatrixTypeAofRewrite(RedisModuleIO *, RedisModuleString *, void *);

void MatrixTypeDigest(RedisModuleDigest *, void *);

void MatrixTypeFree(void *value);

int MatrixTypeRegister(RedisModuleCtx *ctx);

#endif  // __MATRIX-TYPE_H__

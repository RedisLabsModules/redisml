#ifndef __MATRIX_TYPE_H__
#define __MATRIX_TYPE_H__

#include "redismodule.h"
#include "matrix.h"

#define MATRIXTYPE_ENCODING_VERSION 0
#define MATRIXTYPE_NAME "RLML_MTRX"

extern RedisModuleType *MatrixType;

void *MatrixTypeRdbLoad(RedisModuleIO *, int);

void MatrixTypeRdbSave(RedisModuleIO *, void *);

void MatrixTypeAofRewrite(RedisModuleIO *, RedisModuleString *, void *);

void MatrixTypeDigest(RedisModuleDigest *, void *);

void MatrixTypeFree(void *value);

int MatrixTypeRegister(RedisModuleCtx *ctx);

#endif  // __MATRIX-TYPE_H__

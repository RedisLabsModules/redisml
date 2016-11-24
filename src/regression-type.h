#ifndef __REGRESSION_TYPE_H__
#define __REGRESSION_TYPE_H__

#include "redismodule.h"
#include "reg.h"

#define REGRESSIONTYPE_ENCODING_VERSION 0
#define REGRESSIONTYPE_NAME "RLML_RGRS"

extern RedisModuleType *RegressionType;

void *RegressionTypeRdbLoad(RedisModuleIO *, int);

void RegressionTypeRdbSave(RedisModuleIO *, void *);

void RegressionTypeAofRewrite(RedisModuleIO *, RedisModuleString *, void *);

void RegressionTypeDigest(RedisModuleDigest *, void *);

void RegressionTypeFree(void *value);

int RegressionTypeRegister(RedisModuleCtx *ctx);

#endif /*__REGRESSION-TYPE_H__*/

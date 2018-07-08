#ifndef __TF_TYPE_H__
#define __TF_TYPE_H__

#include "redismodule.h"
#include "tf.h"

#define TFTYPE_ENCODING_VERSION 0
#define TFTYPE_NAME "RLML_TEFL"

extern RedisModuleType *TFType;

void *TFTypeRdbLoad(RedisModuleIO *, int);

void TFTypeRdbSave(RedisModuleIO *, void *);

void TFTypeAofRewrite(RedisModuleIO *, RedisModuleString *, void *);

void TFTypeDigest(RedisModuleDigest *, void *);

void TFTypeFree(void *value);

int TFTypeRegister(RedisModuleCtx *ctx);

#endif /*__TF_TYPE_H__*/

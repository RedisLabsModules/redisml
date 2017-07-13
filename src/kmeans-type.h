#ifndef __KMEANS_TYPE_H__
#define __KMEANS_TYPE_H__

#include "redismodule.h"
#include "kmeans.h"

#define KMEANSTYPE_ENCODING_VERSION 0
#define KMEANSTYPE_NAME "RLML_KMNS"

extern RedisModuleType *KmeansType;

void *KmeansTypeRdbLoad(RedisModuleIO *, int);

void KmeansTypeRdbSave(RedisModuleIO *, void *);

void KmeansTypeAofRewrite(RedisModuleIO *, RedisModuleString *, void *);

void KmeansTypeDigest(RedisModuleDigest *, void *);

void KmeansTypeFree(void *value);

int KmeansTypeRegister(RedisModuleCtx *ctx);

#endif /*__KMEANS-TYPE_H__*/

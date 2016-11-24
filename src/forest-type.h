#ifndef __FOREST_TYPE_H__
#define __FOREST_TYPE_H__

#include "redismodule.h"
#include "forest.h"

#define FORESTTYPE_ENCODING_VERSION 0
#define FORESTTYPE_NAME "RLML_FRST"

extern RedisModuleType *ForestType;

void *ForestTypeRdbLoad(RedisModuleIO *, int);

void ForestTypeRdbSave(RedisModuleIO *, void *);

void ForestTypeAofRewrite(RedisModuleIO *, RedisModuleString *, void *);

void ForestTypeDigest(RedisModuleDigest *, void *);

void ForestTypeFree(void *value);

int ForestTypeRegister(RedisModuleCtx *ctx);

#endif /*__FOREST-TYPE_H__*/

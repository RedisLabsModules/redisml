#ifndef __FOREST_TYPE_H__
#define __FOREST_TYPE_H__

#include "redismodule.h"
#include "forest.h"

#define FORESTTYPE_ENCODING_VERSION 0
#define FORESTTYPE_NAME "RLML_FRST"

#define REDIS_ML_FOREST_ERROR_NULL_NODES "Tree contains null nodes" 
#define REDIS_ML_FOREST_ERROR_WRONG_PATH "Tree node path containes wrong characters, should be \'r\' or \'l\'"
#define REDIS_ML_FOREST_ERROR_WRONG_SPLIT_TYPE "Wrong node type , should be \'NUMERIC\', \'CATEGORIC\' or \'LEAF\'"

extern RedisModuleType *ForestType;

void *ForestTypeRdbLoad(RedisModuleIO *, int);

void ForestTypeRdbSave(RedisModuleIO *, void *);

void ForestTypeAofRewrite(RedisModuleIO *, RedisModuleString *, void *);

void ForestTypeDigest(RedisModuleDigest *, void *);

void ForestTypeFree(void *value);

int ForestTypeRegister(RedisModuleCtx *ctx);

#endif /*__FOREST-TYPE_H__*/

#include "redismodule.h"
#include "reg.h"
#include "regression-type.h"

RedisModuleType *RegressionType;

void *RegressionTypeRdbLoad(RedisModuleIO *io, int encver) {
    if (encver != REGRESSIONTYPE_ENCODING_VERSION) {
        return NULL;
    }
    return NULL;
}

void RegressionTypeRdbSave(RedisModuleIO *io, void *ptr) {}

void RegressionTypeAofRewrite(RedisModuleIO *aof, RedisModuleString *key,
                              void *value) {}

void RegressionTypeDigest(RedisModuleDigest *digest, void *value) {}

void RegressionTypeFree(void *value) {}

int RegressionTypeRegister(RedisModuleCtx *ctx) {
    RedisModuleTypeMethods tm = {.version = REDISMODULE_TYPE_METHOD_VERSION,
        .rdb_load = RegressionTypeRdbLoad,
        .rdb_save = RegressionTypeRdbSave,
        .aof_rewrite = RegressionTypeAofRewrite,
        .free = RegressionTypeFree};

    RegressionType = RedisModule_CreateDataType(
            ctx, REGRESSIONTYPE_NAME, 0, &tm);
    if (RegressionType == NULL) {
        return REDISMODULE_ERR;
    }

    return REDISMODULE_OK;
}

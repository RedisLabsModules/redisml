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
    RegressionType = RedisModule_CreateDataType(
            ctx, REGRESSIONTYPE_NAME, 0, RegressionTypeRdbLoad, RegressionTypeRdbSave,
            RegressionTypeAofRewrite, RegressionTypeDigest, RegressionTypeFree);
    if (RegressionType == NULL) {
        return REDISMODULE_ERR;
    }

    return REDISMODULE_OK;
}

#include "redismodule.h"
#include "reg.h"
#include "regression-type.h"

RedisModuleType *RegressionType;

/*

typedef struct {
    double intercept;
    double *coefficients;
    int clen;
} LinReg;
*/

void *RegressionTypeRdbLoad(RedisModuleIO *io, int encver) {
    if (encver != REGRESSIONTYPE_ENCODING_VERSION) {
        return NULL;
    }
    LinReg *r = malloc(sizeof(LinReg));
    r->intercept = RedisModule_LoadDouble(io);
    r->clen = RedisModule_LoadUnsigned(io);
    r->coefficients = calloc(r->clen, sizeof(double));
    for(int i = 0; i < r->clen; i++){
       r->coefficients[i] = RedisModule_LoadDouble(io);
    }
    return r; 
}

void RegressionTypeRdbSave(RedisModuleIO *io, void *ptr) {
    LinReg *r = ptr;
    RedisModule_SaveDouble(io, r->intercept);
    RedisModule_SaveUnsigned(io, r->clen);
    for(int i = 0; i < r->clen; i++){
       RedisModule_SaveDouble(io, r->coefficients[i]);
    }
}

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

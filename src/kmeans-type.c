#include "redismodule.h"
#include "kmeans.h"
#include "kmeans-type.h"

RedisModuleType *KmeansType;

void *KmeansTypeRdbLoad(RedisModuleIO *io, int encver) {
    if (encver != KMEANSTYPE_ENCODING_VERSION) {
        return NULL;
    }
    Kmeans *k = malloc(sizeof(Kmeans));
    k->k = RedisModule_LoadUnsigned(io);
    k->dimensions = RedisModule_LoadUnsigned(io);
    k->centers = calloc(k->k * k->dimensions, sizeof(double));
    for(int i = 0; i < k->k * k->dimensions; i++){
       k->centers[i] = RedisModule_LoadDouble(io);
    }
    return k; 
}

void KmeansTypeRdbSave(RedisModuleIO *io, void *ptr) {
    Kmeans *k = ptr;
    RedisModule_SaveUnsigned(io, k->k);
    RedisModule_SaveUnsigned(io, k->dimensions);
    for(int i = 0; i < k->k * k->dimensions; i++){
       RedisModule_SaveDouble(io, k->centers[i]);
    }
}

void KmeansTypeAofRewrite(RedisModuleIO *aof, RedisModuleString *key,
                              void *value) {}

void KmeansTypeDigest(RedisModuleDigest *digest, void *value) {}

void KmeansTypeFree(void *value) {}

int KmeansTypeRegister(RedisModuleCtx *ctx) {
    RedisModuleTypeMethods tm = {.version = REDISMODULE_TYPE_METHOD_VERSION,
        .rdb_load = KmeansTypeRdbLoad,
        .rdb_save = KmeansTypeRdbSave,
        .aof_rewrite = KmeansTypeAofRewrite,
        .free = KmeansTypeFree};

    KmeansType = RedisModule_CreateDataType(
            ctx, KMEANSTYPE_NAME, 0, &tm);
    if (KmeansType == NULL) {
        return REDISMODULE_ERR;
    }

    return REDISMODULE_OK;
}

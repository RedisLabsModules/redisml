#include "redismodule.h"
#include "tf.h"
#include "tf-type.h"

RedisModuleType *TFType;

//typedef struct {
//    double intercept;
//    double *coefficients;
//    int clen;
//} TF;


void *TFTypeRdbLoad(RedisModuleIO *io, int encver) {
    if (encver != TFTYPE_ENCODING_VERSION) {
        return NULL;
    }
    Tensor *r = malloc(sizeof(Tensor));
    r->intercept = RedisModule_LoadDouble(io);
    r->clen = RedisModule_LoadUnsigned(io);
    r->coefficients = calloc(r->clen, sizeof(double));
    for(int i = 0; i < r->clen; i++){
       r->coefficients[i] = RedisModule_LoadDouble(io);
    }
    return r;
}

void TFTypeRdbSave(RedisModuleIO *io, void *ptr) {
	Tensor *r = ptr;
    RedisModule_SaveDouble(io, r->intercept);
    RedisModule_SaveUnsigned(io, r->clen);
    for(int i = 0; i < r->clen; i++){
       RedisModule_SaveDouble(io, r->coefficients[i]);
    }
}

void TFTypeAofRewrite(RedisModuleIO *aof, RedisModuleString *key,
                              void *value) {}

void TFTypeDigest(RedisModuleDigest *digest, void *value) {}

void TFTypeFree(void *value) {}

int TFTypeRegister(RedisModuleCtx *ctx) {
    RedisModuleTypeMethods tm = {.version = REDISMODULE_TYPE_METHOD_VERSION,
        .rdb_load = TFTypeRdbLoad,
        .rdb_save = TFTypeRdbSave,
        .aof_rewrite = TFTypeAofRewrite,
        .free = TFTypeFree};

    TFType = RedisModule_CreateDataType(ctx, TFTYPE_NAME, 0, &tm);
    if (TFType == NULL) {
        return REDISMODULE_ERR;
    }

    return REDISMODULE_OK;
}

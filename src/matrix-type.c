#include "redismodule.h"
#include "matrix.h"
#include "matrix-type.h"

RedisModuleType *MatrixType;

void *MatrixTypeRdbLoad(RedisModuleIO *io, int encver) {
  if (encver != MATRIXTYPE_ENCODING_VERSION) {
    return NULL;
  }
  return NULL;
}

void MatrixTypeRdbSave(RedisModuleIO *io, void *ptr) {}

void MatrixTypeAofRewrite(RedisModuleIO *aof, RedisModuleString *key,
                          void *value) {}

void MatrixTypeDigest(RedisModuleDigest *digest, void *value) {}

void MatrixTypeFree(void *value) {}

int MatrixTypeRegister(RedisModuleCtx *ctx) {
  MatrixType = RedisModule_CreateDataType(
      ctx, MATRIXTYPE_NAME, 0, MatrixTypeRdbLoad, MatrixTypeRdbSave,
      MatrixTypeAofRewrite, MatrixTypeDigest, MatrixTypeFree);
  if (MatrixType == NULL) {
    return REDISMODULE_ERR;
  }

  return REDISMODULE_OK;
}

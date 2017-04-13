#include "redismodule.h"
#include "matrix.h"
#include "matrix-type.h"

RedisModuleType *MatrixType;

void *MatrixTypeRdbLoad(RedisModuleIO *io, int encver) {
    if (encver != MATRIXTYPE_ENCODING_VERSION) {
        return NULL;
    }
    Matrix *m = malloc(sizeof(Matrix));
    m->rows = RedisModule_LoadUnsigned(io);
    m->cols = RedisModule_LoadUnsigned(io);
    m->values = calloc(m->rows * m->cols, sizeof(double));
    for(int i = 0; i < m->rows * m->cols; i++){
       m->values[i] = RedisModule_LoadDouble(io);
    }
    return m; 
}

void MatrixTypeRdbSave(RedisModuleIO *io, void *ptr) {
    Matrix *m = ptr;
    RedisModule_SaveUnsigned(io, m->rows);
    RedisModule_SaveUnsigned(io, m->cols);
    for(int i = 0; i < m->rows * m->cols; i++){
       RedisModule_SaveDouble(io, m->values[i]);
    } 
}

void MatrixTypeAofRewrite(RedisModuleIO *aof, RedisModuleString *key, void *value) {
    Matrix *m = value;
    RedisModule_EmitAOF(aof,"ML.MATRIX.SET", "sll", key, m->rows, m->cols );
    for(int i = 0; i < m->rows * m->cols; i++){
        RedisModule_EmitAOF(aof,"ML.MATRIX.SET", "d", key, i / m->cols, i % m->cols, m->values[i]);
    } 
}

void MatrixTypeDigest(RedisModuleDigest *digest, void *value) {}

void MatrixTypeFree(void *value) {}

int MatrixTypeRegister(RedisModuleCtx *ctx) {

    RedisModuleTypeMethods tm = {.version = REDISMODULE_TYPE_METHOD_VERSION,
        .rdb_load = MatrixTypeRdbLoad,
        .rdb_save = MatrixTypeRdbSave,
        .aof_rewrite = MatrixTypeAofRewrite,
        .free = MatrixTypeFree};

    MatrixType = RedisModule_CreateDataType(ctx, MATRIXTYPE_NAME, 0, &tm);
    if (MatrixType == NULL) {
        return REDISMODULE_ERR;
    }

    return REDISMODULE_OK;
}

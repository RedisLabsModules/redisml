#include "redismodule.h"
#include "forest.h"
#include "forest-type.h"

RedisModuleType *ForestType;

void *ForestTypeRdbLoad(RedisModuleIO *io, int encver) {
    if (encver != FORESTTYPE_ENCODING_VERSION) {
        return NULL;
    }
    Forest *f;
    f = malloc(sizeof(*f));
    f->len = RedisModule_LoadUnsigned(io);
    f->Trees = malloc(f->len * sizeof(Forest_Tree *));

    for (int i = 0; i < f->len; i++) {
        Forest_Tree *t = malloc(sizeof(Forest_Tree));
        f->Trees[i] = t;
        size_t tlen;
        char *s = RedisModule_LoadStringBuffer(io, &tlen);
        Forest_TreeDeSerialize(s, &t->root, tlen);
    }
    return f;
}

void ForestTypeRdbSave(RedisModuleIO *io, void *ptr) {
    Forest *f = ptr;
    RedisModule_SaveUnsigned(io, f->len);
    for (int i = 0; i < f->len; i++) {
        char path = 0;
        char *s = NULL;
        int len = Forest_TreeSerialize(&s, f->Trees[i]->root, &path, 0, 0);
        RedisModule_SaveStringBuffer(io, s, len);
        free(s);
    }
}

void ForestTypeAofRewrite(RedisModuleIO *aof, RedisModuleString *key,
                          void *value) {}

void ForestTypeDigest(RedisModuleDigest *digest, void *value) {}

void ForestTypeFree(void *value) {
    Forest *f = value;
    for (int i = 0; i < f->len; i++) {
        Forest_TreeDel(f->Trees[i]->root);
    }
    free(f);
}

int ForestTypeRegister(RedisModuleCtx *ctx) {
    RedisModuleTypeMethods tm = {.version = REDISMODULE_TYPE_METHOD_VERSION,
        .rdb_load = ForestTypeRdbLoad,
        .rdb_save = ForestTypeRdbSave,
        .aof_rewrite = ForestTypeAofRewrite,
        .free = ForestTypeFree};


    ForestType = RedisModule_CreateDataType(
            ctx, FORESTTYPE_NAME, 0, &tm);
    if (ForestType == NULL) {
        return REDISMODULE_ERR;
    }

    return REDISMODULE_OK;
}


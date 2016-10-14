#ifndef __RM_ALLOC__
#define __RM_ALLOC__

#include <stdlib.h>

#ifdef IS_REDIS_MODULE
#include "redismodule.h"

/* Explicitly override malloc, calloc, realloc & free with RedisModule_. */
#define malloc(size) RedisModule_Alloc(size)
#define calloc(count, size) RedisModule_Calloc(count, size)
#define realloc(ptr, size) RedisModule_Realloc(ptr, size)
#define free(ptr) RedisModule_Free(ptr)

/* More overriding */
// needed to avoid calling strndup->malloc
#define strndup(s,n) rmalloc_strndup(s,n)
char *rmalloc_strndup(const char *s, size_t n);
#endif /* IS_REDIS_MODULE */

#endif /* __RM_ALLOC__ */
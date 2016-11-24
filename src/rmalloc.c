#include "rmalloc.h"
#include <string.h>

char *rmalloc_strndup(const char *s, size_t n) {
    char *ret = calloc(n + 1, sizeof(char));
    if (ret)
        memcpy(ret, s, n);
    return ret;
}

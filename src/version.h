#ifndef REDISML_VERSION_H_
/* This is where the modules build/version is declared.
 *  If declared with -D in compile time, this file is ignored
 */

#ifndef REDISML_VERSION_MAJOR
#define REDISML_VERSION_MAJOR 0
#endif

#ifndef REDISML_VERSION_MINOR
#define REDISML_VERSION_MINOR 99
#endif

#ifndef REDISML_VERSION_PATCH
#define REDISML_VERSION_PATCH 1
#endif

#define REDISML_MODULE_VERSION \
    (REDISML_VERSION_MAJOR * 10000 + REDISML_VERSION_MINOR * 100 + REDISML_VERSION_PATCH)

#endif

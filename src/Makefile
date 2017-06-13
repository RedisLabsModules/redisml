#set environment variable RM_INCLUDE_DIR to the location of redismodule.h
ifndef RM_INCLUDE_DIR
	RM_INCLUDE_DIR=.
endif

ifndef RMUTIL_LIBDIR
	RMUTIL_LIBDIR=rmutil
endif

# find the OS
uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')

# Compile flags for linux / osx
ifeq ($(uname_S),Linux)
	SHOBJ_CFLAGS ?=  -fno-common -g -ggdb
	SHOBJ_LDFLAGS ?= -shared -Bsymbolic
else
	SHOBJ_CFLAGS ?= -dynamic -fno-common -g -ggdb
	SHOBJ_LDFLAGS ?= -bundle -undefined dynamic_lookup
endif
CFLAGS = -I$(RM_INCLUDE_DIR) -Wall -g -fPIC -lc -lm -l:libcblas.a -l:libatlas.a -O2 -std=gnu99 -pthread 
CC=gcc

OBJS=rmalloc.o forest.o forest-type.o feature-vec.o reg.o regression-type.o matrix.o matrix-type.o util/logging.o util/thpool.o

.PHONY: all
all: redis-ml.so  

redis-ml.so: rmutil/librmutil.a redis-ml.o $(OBJS)
	$(LD) -o $@ redis-ml.o $(OBJS) $(SHOBJ_LDFLAGS) -l:libcblas.a -l:libatlas.a $(LIBS) -L$(RMUTIL_LIBDIR) -lrmutil -lc 

rmutil/librmutil.a:
	echo "making rmutil" && cd rmutil && $(MAKE) all

rmalloc.o: rmalloc.h
forest.o: forest.h
forest-type.o: forest-type.h
feature-vec.o: feature-vec.h
reg.o: reg.h
regression-type.o: regression-type.h
matrix.o: matrix.h
matrix-type.o: matrix-type.h

.PHONY: test
test:
	$(MAKE) -C ./test clean all test
	$(MAKE) -C ./test/pytest test

.PHONY: clean
clean:
	rm -rf *.xo *.so *.o

package: redis-ml.so
		mkdir -p ../build
		module_packer -v -o "../build/redis-ml.{os}-{architecture}.latest.zip" "`pwd`/redis-ml.so"


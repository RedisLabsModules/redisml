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
	SHOBJ_LDFLAGS ?= -shared -Bsymbolic -l:libcblas.a -l:libatlas.a 
else
	SHOBJ_CFLAGS ?= -dynamic -fno-common -g -ggdb
	SHOBJ_LDFLAGS ?= -bundle -undefined dynamic_lookup -framework Accelerate
	BLAS_INCLUDE ?= /System/Library/Frameworks/Accelerate.framework/Versions/Current/Frameworks/vecLib.framework/Headers/
endif

CFLAGS = -I$(RM_INCLUDE_DIR) -Wall -g -fPIC -O2 -std=gnu99 -pthread 
CC=gcc

OBJS=rmalloc.o forest.o forest-type.o feature-vec.o reg.o regression-type.o matrix.o matrix-type.o kmeans.o kmeans-type.o util/logging.o util/thpool.o

MODULE_ARTIFACT=redis-ml.so

.PHONY: all
all: redis-ml.so  

redis-ml.so: rmutil/librmutil.a redis-ml.o $(OBJS)
	$(LD) -o $@ redis-ml.o $(OBJS) $(SHOBJ_LDFLAGS) $(LIBS) -L$(RMUTIL_LIBDIR) -lrmutil -lc 

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
kmeans.o: kmeans.h
kmeans-type.o: kmeans-type.h

.PHONY: test
test:
	$(MAKE) -C ./test clean all test
	$(MAKE) -C ./test/pytest test

.PHONY: clean
clean:
	rm -rf *.xo *.so *.o
	$(MAKE) -C ./rmutil clean
	$(MAKE) -C ./test clean
	$(MAKE) -C ./util clean

include Makefile.package

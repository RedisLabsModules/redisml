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
CFLAGS = -I$(RM_INCLUDE_DIR) -Wall -g -fPIC -lc -lm -lcblas -O2 -std=gnu99  
CC=gcc

OBJS=rmalloc.o tree.o forest-type.o reader.o reg.o regression-type.o matrix.o matrix-type.o util/logging.o

all: redis-ml.so  

redis-ml.so: redis-ml.o $(OBJS)
	$(LD) -o $@ redis-ml.o $(OBJS) $(SHOBJ_LDFLAGS) $(LIBS) -L$(RMUTIL_LIBDIR) -lrmutil -lc -lcblas

clean:
	rm -rf *.xo *.so *.o


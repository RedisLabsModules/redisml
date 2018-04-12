ifeq ($(USE_BLAS), 1) 
	BLAS_FLAGS = -l:libcblas.a -l:libatlas.a -lpthread -DUSE_BLAS
endif

CFLAGS = -g -O3 -fPIC -D_GNU_SOURCE -std=gnu99 -I./ 
CC=gcc

SOURCEDIR=$(abspath $(shell pwd -P)/../)
#C_SOURCES = $(wildcard $(SOURCEDIR)/*.c)
C_SOURCES = $(filter-out $(SOURCEDIR)/input.c, $(wildcard $(SOURCEDIR)/*.c))
OBJS = $(patsubst %.c, %.o, $(C_SOURCES) )

TEST_SOURCES = $(abspath $(wildcard test_*.c))
TEST_OBJECTS = $(patsubst %.c, %.o, $(TEST_SOURCES))
TEST_EXECUTABLES = $(patsubst %.c, %.run, $(TEST_SOURCES))

%.o: %.c
		$(CC) $(CFLAGS) $(BLAS_FLAGS) -c $< -o $@ 

%.run: %.c
		$(CC) $(CFLAGS) $(OBJS) $< -o $@ -DTEST -lm $(BLAS_FLAGS)

all: objs build test

objs:
ifeq ($(USE_BLAS), 1) 
	$(MAKE) -C ../ clean USE_BLAS=1 all
else
	$(MAKE) -C ../ clean all
endif
build: $(TEST_EXECUTABLES)

test: build $(TEST_EXECUTABLES)
		set -e; \
		for t in test_*.run;\
			do ./$$t;\
		done


clean:
		-rm -f *.o *.run

.PHONY: clean

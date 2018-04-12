ifeq ($(USE_BLAS), 1) 
	BLAS_FLAGS = -pthread -l:libcblas.a -l:libatlas.a -DUSE_BLAS
endif

CFLAGS = -g -O3 -fPIC -D_GNU_SOURCE -std=gnu99 -I./
LDFLAGS = -lm $(BLAS_FLAGS)
CC=gcc

SOURCEDIR=$(abspath $(shell pwd -P)/.)
C_SOURCES = $(filter-out $(SOURCEDIR)/input.c, $(wildcard $(SOURCEDIR)/*.c))
OBJS = $(patsubst %.c, %.o, $(C_SOURCES) )

%.o: %.c %.h
		$(CC) $(CFLAGS) $(BLAS_FLAGS) -c $< -o $@ 


all: input


input: $(OBJS) input.c 
	$(CC) $(CFLAGS) $@.c $(OBJS) $(LDFLAGS) -o $@


info:
	$(info $(C_SOURCES))

.PHONY: info


clean:
		-rm -f *.o

.PHONY: clean

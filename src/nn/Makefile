ifeq ($(USE_BLAS), 1) 
	BLAS_FLAGS = -l:libcblas.a -l:libatlas.a -DUSE_BLAS
endif

CFLAGS = -g -O3 -fPIC -D_GNU_SOURCE -std=gnu99 -I./
CC=gcc

SOURCEDIR=$(abspath $(shell pwd -P)/.)
C_SOURCES = $(wildcard $(SOURCEDIR)/*.c) 
OBJS = $(patsubst %.c, %.o, $(C_SOURCES) )

%.o: %.c %.h
		$(CC) $(CFLAGS) $(BLAS_FLAGS) -c $< -o $@ 


all: $(OBJS)


info:
	$(info $(CC_SOURCES))

.PHONY: info


clean:
		-rm -f *.o

.PHONY: clean

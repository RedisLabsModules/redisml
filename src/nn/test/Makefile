CFLAGS = -g -O3 -fPIC -D_GNU_SOURCE -std=gnu99 -I./ 

CC=gcc

SOURCEDIR=$(abspath $(shell pwd -P)/../)
C_SOURCES = $(wildcard $(SOURCEDIR)/*.c) 
OBJS = $(patsubst %.c, %.o, $(C_SOURCES) )

TEST_SOURCES = $(abspath $(wildcard test_*.c))
TEST_OBJECTS = $(patsubst %.c, %.o, $(TEST_SOURCES))
TEST_EXECUTABLES = $(patsubst %.c, %.run, $(TEST_SOURCES))

%.o: %.c
		$(CC) $(CFLAGS) -c $< -o $@ 

%.run: %.c
		$(CC) $(CFLAGS) $(OBJS) $< -o $@ -DTEST

all: objs build test

objs:
	$(MAKE) -C ../ all

build: $(TEST_EXECUTABLES)

test: build $(TEST_EXECUTABLES)
		set -e; \
		for t in test_*.run;\
			do ./$$t;\
		done


clean:
		-rm -f *.o *.run

.PHONY: clean

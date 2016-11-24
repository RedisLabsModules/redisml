CFLAGS = -Wall -g -fPIC -lc -O2 -std=gnu99
CC=gcc

OBJS=../tree.o ../feature-vec.o ../util/logging.o

all: tree_test feature-vec_test

tree_test: tree_test.c $(OBJS)
	$(CC) $@.c $(OBJS) -o $@ 

feature-vec_test: feature-vec_test.c $(OBJS)
	$(CC) $@.c $(OBJS) -o $@ 

clean:
	rm -rf tree_test feature-vec_test *.xo *.so *.o


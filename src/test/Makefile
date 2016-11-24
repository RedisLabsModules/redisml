CFLAGS = -Wall -g -fPIC -lc -O2 -std=gnu99
CC=gcc

OBJS=../forest.o ../feature-vec.o ../util/logging.o

all: forest_test feature-vec_test

forest_test: forest_test.c $(OBJS)
	$(CC) $@.c $(OBJS) -o $@ 

feature-vec_test: feature-vec_test.c $(OBJS)
	$(CC) $@.c $(OBJS) -o $@ 

clean:
	rm -rf forest_test feature-vec_test *.xo *.so *.o


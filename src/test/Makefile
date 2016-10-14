CFLAGS = -Wall -g -fPIC -lc -O2 -std=gnu99
CC=gcc

OBJS=../tree.o ../reader.o ../util/logging.o

all: tree_test reader_test

tree_test: tree_test.c $(OBJS)
	$(CC) $@.c $(OBJS) -o $@ 

reader_test: reader_test.c $(OBJS)
	$(CC) $@.c $(OBJS) -o $@ 

clean:
	rm -rf tree_test reader_test *.xo *.so *.o


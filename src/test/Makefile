CFLAGS = -Wall -g -fPIC -lc -pthread -O2 -std=gnu99
CC=gcc

OBJS=../forest.o ../feature-vec.o ../util/logging.o ../util/thpool.o

all: forest_test feature-vec_test

forest_test: forest_test.c $(OBJS)
	$(CC) $@.c $(OBJS) $(CFLAGS) -o $@ 

feature-vec_test: feature-vec_test.c $(OBJS)
	$(CC) $@.c $(OBJS) $(CFLAGS) -o $@ 

.PHONY: test_forest
test_forest:
	@(sh -c ./forest_test)

.PHONY: test_feature-vec
test_feature-vec:
	@(sh -c ./feature-vec_test)

.PHONY: test
test: test_forest test_feature-vec

clean:
	rm -rf forest_test feature-vec_test *.xo *.so *.o


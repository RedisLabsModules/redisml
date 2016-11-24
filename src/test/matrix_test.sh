#!/usr/bin/env bash

if pgrep redis-server > /dev/null
then
    echo "Trying with running Redis"
else
    echo "Starting Redis"
    redis-server --loadmodule ../redis-ml.so &
    sleep 1
fi

redis-cli ml.matrix.set a 2 3 1 2 5 3 4 6

redis-cli ml.matrix.set b 3 2 1 2 3 4 7 1

redis-cli ml.matrix.multiply a b c
redis-cli ml.matrix.multiply a b c

redis-cli ml.matrix.get c

pkill redis-server

#1) (integer) 2
#2) (integer) 2
#3) "42"
#4) "15"
#5) "57"
#6) "28"

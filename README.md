# Redis-ML

### Machine Learning Model Server on Redis 

## Overview

Redis-ML is a Redis module that implements several machine learning models as Redis data types.

The stored models are fully operational and support performing the prediction/evaluation.

Redis-ML is a turn key solution for using trained models in a production environment. Allowing loading ML models from any platform immediately ready to serve.

## Primary Features:

* Decision Tree ensembles classification and regression.
* Linear regression.
* Logistic regression.
* Matrix operations.

  ​

## Building and running:

```sh
git clone https://github.com/RedisLabsModules/redis-ml.git
cd redis-ml/src
make 
```

## Assuming you have a redis build from the unstable branch:
```
/path/to/redis-server --loadmodule ./redis-ml.so
```

## Commands:

* ml.forest.add - Add trees/nodes to a forest

  ml.forest.run <forest> <data_item> [CLASSIFICATION|REGRESSION]

* ml.forest.run - Classification/regression of a data item

  ml.forest.run <forest> <data_item> [CLASSIFICATION|REGRESSION]

forest example:

```sh
127.0.0.1:6379> ml.forest.add f1 0 . numeric 1 0.1 .l leaf 1 .r leaf 0
OK
127.0.0.1:6379> ml.forest.add f1 1 . numeric 1 0.1 .l leaf 1 .r leaf 0
OK
127.0.0.1:6379> ml.forest.add f1 2 . numeric 1 0.1 .l leaf 0 .r leaf 1
OK
127.0.0.1:6379> ml.forest.run "f1" "1:0.01" CLASSIFICATION
"1"
127.0.0.1:6379> ml.forest.run "f1" "1:0.2" CLASSIFICATION
"0"

```



* ml.linreg.set - Initialize/update a linear regression predictor item
* ml.linreg.predict - Predict result for a feature set 

linreg example:

```sh
127.0.0.1:6379> ml.linreg.set a 2 3 4 5
OK
127.0.0.1:6379> ml.linreg.predict a 1 1 1
"14"
```



* ml.logreg.set - Initialize/update a logistic regression classifier item
* ml.logreg.predict - Predict class probability for a feature set 

logreg example:

```sh
127.0.0.1:6379> ml.logreg.set a 0 2 2
OK
127.0.0.1:6379> ml.logreg.predict  a -3 1
"0.017986209962091559"
```



* ml.matrix.set - Initialize/update a matrix
* ml.matrix.multiply - Multiply two existing matrices into a new one
* ml.matrix.add - Add two existing matrices into a new one
* ml.matrix.scale - Multiply
* ml.matrix.print - Print a matrix to the Redis server output (for debugging)

matrix example:

```sh
127.0.0.1:6379> ml.matrix.set a 2 3 1 2 5 3 4 6
OK
127.0.0.1:6379> ml.matrix.set b 3 2 1 2 3 4 7 1
OK
127.0.0.1:6379> ml.matrix.multiply a b c
OK
127.0.0.1:6379> ml.matrix.print c
OK
127.0.0.1:6379> ml.matrix.get c
1) (integer) 2
2) (integer) 2
3) "42"
4) "15"
5) "57"
6) "28"
```



### License: AGPL


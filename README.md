[![CircleCI](https://circleci.com/gh/RedisLabsModules/redis-ml.svg?style=svg)](https://circleci.com/gh/RedisLabsModules/redis-ml)

# Machine Learning Model Server on Redis

## Overview

Redis-ML is a Redis module that implements several machine learning models as Redis data types.

The stored models are fully operational and support the prediction/evaluation process.

Redis-ML is a turnkey solution for using trained models in a production environment. Load ML models from any platform, immediately ready to serve.

See Full Documentation at http://redisml.io

## Primary Features

* Decision Tree ensembles (random forests) classification and regression
* Linear regression
* Logistic regression
* Matrix operations

## Building and Running

- Build a Redis server with support for modules (currently available from the [unstable branch](https://github.com/antirez/redis/tree/unstable)).

- You'll also need a [BLAS](http://www.netlib.org/blas/) library such as [ATLAS](http://math-atlas.sourceforge.net/). To install ATLAS:

  - Ubuntu: 
  ```sh
  sudo apt-get install libatlas-base-dev
  ```
  
  - CentOS/RHEL/Fedora: 
  ```sh
  sudo yum install -y atlas-devel atlas-static
  ln -s /usr/lib64/atlas/libatlas.a /usr/lib64/libatlas.a 
  ln -s /usr/lib64/atlas/libtatlas.so /usr/lib64/libcblas.a 
  ```

- Build the Redis-ML module:

  ```sh
  git clone https://github.com/RedisLabsModules/redis-ml.git
  cd redis-ml/src
  make
  ```

- To load the module, start Redis with the `--loadmodule /path/to/redis-ml/src/redis-ml.so` option, add it as a directive to the configuration file or send a `MODULE LOAD` command.

# Redis ML Commands

## **Decision tree ensembles**

### Example of use

The following code creates a [random forest](https://en.wikipedia.org/wiki/Random_forest) under the key `myforest` that consists of three trees with IDs ranging from 0 to 2, where each consists of a single numeric splitter and its predicate values. Afterwards, the forest is used to classify two inputs and yield their predictions.

```
redis> ML.FOREST.ADD myforest 0 . NUMERIC 1 0.1 .l LEAF 1 .r LEAF 0
OK
redis> ML.FOREST.ADD myforest 1 . NUMERIC 1 0.1 .l LEAF 1 .r LEAF 0
OK
redis> ML.FOREST.ADD myforest 2 . NUMERIC 1 0.1 .l LEAF 0 .r LEAF 1
OK
redis> ML.FOREST.RUN myforest 1:0.01 CLASSIFICATION
"1"
redis> ML.FOREST.RUN myforest 1:0.2 CLASSIFICATION
"0"
```

### ML.FOREST.ADD

> **Available since 1.0.0.**  
> **Time complexity:** O(M*log(N)) where N is the tree's depth and M is the number of nodes added

#### Syntax
```
ML.FOREST.ADD key tree path ((NUMERIC|CATEGORIC) attr val | LEAF val [STATS]) [...]
```
#### Description
Add nodes to a tree in the forest.
This command adds one or more nodes to the tree in the forest that's stored under `key`. Trees are identified by numeric IDs, `treeid`, that must begin at 0 and be incremented by exactly 1 for each new tree. 

Each of the nodes is described by its path and definition. The `path` argument is the path from the tree's root to the node. A valid path always starts with the period character (`.`), which denotes the root. Optionally, the root may be followed by left or right branches, denoted by the characters `l` and `r`, respectively. For example, the path _".lr"_ refers to the right child of the root's left child.

A node in the decision tree can either be a splitter or a terminal leaf.  Splitter nodes are either numerical or categorical, and are added using the `NUMERIC` or `CATEGORIC` keywords. Splitter nodes also require specifying the examined attribute (`attr`) as well as the value (`val`) used in the comparison made during the branching decision. `val` is expected to be a double-precision floating point value for numerical splitters, and a string for categorical splitter nodes.

The leaves are created with the `LEAF` keyword and only require specifying their double-precision floating point value (`val`).

#### Return value:
Simple string reply

### ML.FOREST.RUN

> **Available since 1.0.0.**  
> **Time complexity:** O(M*log(N)) where N is the depth of the trees and M is the number of trees in the forest

#### Syntax
```
ML.FOREST.RUN key sample (CLASSIFICATION|REGRESSION)
```
#### Description
Predicts the classified (discrete) or regressed (continuous) value of a sample using the forest.
The forest that's stored in `key` is used for generating the predicted value for the `sample`. The sample is given as a string that is a vector of attribute-value pairs in the format of `attr:val`. For example,  the `sample` _"gender:male"_ has a single attribute, _gender_, whose value is _male_. A sample may have multiple such attribute-value pairs, and these must be comma-separated (`,`) in the string vector. For example, a sample of a 25-years-old male is expressed as _"gender:male,age:25"_.

#### Return value:
Bulk string reply: the predicted value of the sample

## **Linear regression**

### Example of use
The first line of the example shows how a linear regression predictor is set to the key named `linear`. The predictor has an intercept of 2 and its coefficients are 3, 4 and 5. Once the predictor is ready, it is used to predict the result given the independent variables' values (features) of 1, 1 and 1.

```
redis> ML.LINREG.SET linear 2 3 4 5
OK
redis> ML.LINREG.PREDICT linear 1 1 1
"14"
```

### ML.LINREG.SET

> **Available since 1.0.0.**  
> **Time complexity:** O(N) where N is the number of coefficients

#### Syntax
```
ML.LINREG.SET key intercept coefficient [...]
```
#### Description
Sets a linear regression predictor.
This command creates or updates the linear regression predictor that's stored in `key`. The predictor's intercept is specified by  `intercept`, followed by one or more `coefficient` arguments of the independent variables.

#### Return value:
Simple string reply

### ML.LINREG.PREDICT

> **Available since 1.0.0.**  
> **Time complexity:** O(N) where N is the number of features

#### Syntax
```
ML.LINREG.PREDICT key feature [...]
```
#### Description
Predicts the result for a set of features.
The linear regression predictor stored in `key` is used for predicting the result based on one or more features that are given by the `feature` argument(s).

#### Return value:
Bulk string reply: the predicted result for the feature set

## **Logistic regression**

### Example of use

In this example, the first line shows how a logistic regression predictor is set to the key named `logistic`. The predictor has an intercept of 0 and its coefficients are 2 and 2. Once the predictor is ready, it is used to predict the result given the independent variables' values (features) of -3 and 1.

```
redis> ML.LOGREG.SET logistic 0 2 2
OK
redis> ML.LOGREG.PREDICT logistic -3 1
"0.017986209962091559"
```

### ML.LOGREG.SET

> **Available since 1.0.0.**  
> **Time complexity:** O(N) where N is the number of coefficients

#### Syntax
```
ML.LOGREG.SET key intercept coefficient [...]
```
#### Description
Sets a linear regression predictor.
This command sets or updates the logistic regression predictor that's stored in `key`. The predictor's intercept is specified by  `intercept`, followed by one or more `coefficient` arguments of the independent variables.

#### Return value:
Simple string reply

### ML.LOGREG.PREDICT

> **Available since 1.0.0.**  
> **Time complexity:** O(N) where N is the number of features

#### Syntax
```
ML.LOGREG.PREDICT key feature [...]
```
#### Description
Predicts the result for a set of features.
The logistic regression predictor stored in `key` is used for predicting the result based on one or more features that are given by the `feature` argument(s).

#### Return value:
Bulk string reply: the predicted result for the feature set

## **Matrix operations**

#### Example of use

The following example shows how to set two matrices, `a` and `b`, multiply them, and store the result in the matrix `ab`. Lastly, the contents of `ab` are fetched.

```
redis> ML.MATRIX.SET a 2 3 1 2 5 3 4 6
OK
redis> ML.MATRIX.SET b 3 2 1 2 3 4 7 1
OK
redis> ML.MATRIX.MULTIPLY a b ab
OK
redis> ML.MATRIX.GET ab
1) (integer) 2
2) (integer) 2
3) "42"
4) "15"
5) "57"
6) "28"
```

### ML.MATRIX.SET

> **Available since 1.0.0.**  
> **Time complexity:** O(N*M) where N is the number of rows and M is the number of columns

#### Syntax
```
ML.MATRIX.SET key n m entry11 .. entrynm
```
#### Description
Sets a matrix.
Sets `key` to store a matrix of  `n` rows,`m` columns and double-precision float entries ranging from `entry11` to `entrynm`.

#### Return value:

Simple string reply

### ML.MATRIX.GET

> **Available since 1.0.0.**  
> **Time complexity:** O(N*M) where N is the number of rows and M is the number of columns

#### Syntax
```
ML.MATRIX.GET key
```
#### Description
Get a matrix.
Returns the matrix's dimensions and entries.

#### Return value:
The first two elements in the returned array are the matrix's rows and columns, respectively, followed by the entries.

### ML.MATRIX.ADD

> **Available since 1.0.0.**  
> **Time complexity:** O(N*M) where N is the number of rows and M is the number of columns

#### Syntax
```
ML.MATRIX.ADD matrix1 matrix2 sum
```
#### Description
Adds matrices.
The result of adding the two matrices stored in `matrix1` and `matrix2` is set in `sum`.

#### Return value:
Simple string reply

### ML.MATRIX.MULTIPLY

> **Available since 1.0.0.**  
> **Time complexity:** O(N\*M\*P) where N and M are numbers of rows and columns in `matrix1`, and P is the number of columns in `matrix2`

#### Syntax
```
ML.MATRIX.MULTIPLY matrix1 matrix2 product
```
#### Description
Multiplies matrices.
The result of multiplying the two matrices stored in `matrix1` and `matrix2` is set in `product`.

#### Return value:

Simple string reply

### ML.MATRIX.SCALE

> **Available since 1.0.0.**  
> **Time complexity:** O(N*M) where N is the number of rows and M is the number of columns

#### Syntax
```
ML.MATRIX.SCALE key scalar
```
#### Description
Scales a matrix.
Updates the entries of the matrix stored in `key` by multiplying them with `scalar`.

#### Return value:
Simple string reply

## **K-means**

### Example of use
Setting up a K-means model in key `k` with 2 clusters and 3 dimensions. The cluster centers are `1, 1, 2` and `2, 5, 4`:

```
redis> ML.KMEANS.SET k 2 3 1 1 2 2 5 4
OK
```

Predicting the cluster of feature vector `1, 3, 5`:
```
redis> ML.KMEANS.predict k 1 3 5 
(integer) 1
```

### ML.KMEANS.SET

> **Available since 1.0.0.**  
> **Time complexity:** O(N) where N is the number of coefficients

#### Syntax
```
ML.KMEANS.SET key k dimensions centers [...]
```
#### Description
Create/update a K-means model.
This command creates or updates the K-means model that's stored in `key`. The number of classes is specified by  `k`, the number of features is set by `dimensions` .

#### Return value:
Simple string reply

### ML.KMEANS.PREDICT

> **Available since 1.0.0.**  
> **Time complexity:** O(N) where N is the number of features

#### Syntax
```
ML.KMEANS.PREDICT key feature [...]
```
#### Description
Predicts the result for a set of features.
The K-means model stored in `key` is used for predicting the result based on one or more features that are given by the `feature` argument(s).

#### Return value:
Integer reply: the predicted result for the feature set

## Contributing

Issue reports, pull and feature requests are welcome.

## License

AGPLv3 - see [LICENSE](LICENSE)

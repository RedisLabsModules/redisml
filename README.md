# Redis-ML

### Machine Learning Model Server 

# Overview

Redis-ML implements several machine learning models as Redis data types.

The stored models are fully operational and support performing the prediction/evaluation.

Redis-ML is a turn key solution for using trained models in a production environment. Allowing loading ML models from any platform immediately ready to serve.

## Primary Features:

* Decision Tree ensembles classification and regression.
* Linear regression.
* Logistic regression.
* Matrix operations.

  ​

### Not *yet* supported:

* Neural networks.
* SGD.

  ​

### License: AGPL

Which basically means you can freely use this for your own projects without "virality" to your code,
as long as you're not modifying the module itself.

## Building and running:

```sh
git clone https://github.com/RedisLabsModules/redis-ml.git
cd redis-ml/src
make 
```

# Assuming you have a redis build from the unstable branch:
```
/path/to/redis-server --loadmodule ./redis-ml.so
```

## API:


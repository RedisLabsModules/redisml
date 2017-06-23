# Machine Learning Model Server on Redis

Redis-ML is a Redis module that implements several machine learning models as Redis data types.

The stored models are fully operational and support performing the prediction/evaluation.

Redis-ML is a turn key solution for using trained models in a production environment. Allowing loading ML models from any platform immediately ready to serve.

The module includes these primary features:

* Decision Tree ensembles (random forests) classification and regression
* Linear regression
* Logistic regression
* Matrix operations

## Quickstart

1.  [Build the redis-ml module library](#building-the-module)
1.  [Load Redis-ML to Redis](#loading-the-module-to-redis)
1.  [Use it from **any** Redis client](#using-redis-ml)

## Building the module

### Linux Ubuntu 16.04

Requirements:

* The `build-essential` package: `apt-get install build-essential`
* You'll also need a [BLAS](http://www.netlib.org/blas/) library, for example [ATLAS](http://math-atlas.sourceforge.net/). To install ATLAS: `apt-get install libatlas-base-dev`

Build the Redis-ML module:

```sh
git clone https://github.com/RedisLabsModules/redis-ml.git
cd redis-ml/src
make
```

Congratulations! You can find the compiled module library at `src/redis-ml.so`.

### RedHat Enterprise Linux/Fedora/CentOS

Requirements:

* The `Development Tools` group: `yum groupinstall 'Development Tools'`
* You'll also need a [BLAS](http://www.netlib.org/blas/) library, for example [ATLAS](http://math-atlas.sourceforge.net/). To install ATLAS: `sudo yum install atlas-devel`

Build the Redis-ML module:

```sh
git clone https://github.com/RedisLabsModules/redis-ml.git
cd redis-ml/src
make
```

Congratulations! You can find the compiled module library at `src/redis-ml.so`.

## Loading the module to Redis

Requirements:

* [Redis v4.0 or above](http://redis.io/download)

We recommend you have Redis load the module during startup by adding the following to your `redis.conf` file:

```
loadmodule /path/to/module/redis-ml.so
```

In the line above replace `/path/to/module/redis-ml` with the actual path to the module's library. Alternatively, you can have Redis load the module using the following command line argument syntax:

```bash
~/$ redis-server --loadmodule /path/to/module/redis-ml.so
```

Lastly, you can also use the [`MODULE LOAD`](http://redis.io/commands/module-load) command. Note, however, that `MODULE LOAD` is a **dangerous command** and may be blocked/deprecated in the future due to security considerations.

Once the module has been loaded successfully, the Redis log should have lines similar to:

```
...
11867:M 23 Jun 01:03:54.750 * Module 'redis-ml' loaded from <redacted>/src/rejson.so
...
```

## Using Redis-ML

Before using Redis-ML, you should familiarize yourself with its commands and syntax as detailed in the [commands reference](commands) document.

You can call Redis-ML's commands from any Redis client.

### With `redis-cli`

```sh
$ redis-cli
127.0.0.1:6379> ML.FOREST.ADD myforst 0 . NUMERIC 1 0.1 .l LEAF 1 .r LEAF 0
OK
```

### With any other client

You can call the module's APU using your client's ability to send raw Redis commands. Depending on your client of choice, the exact method for doing that may vary.

#### Python example

This code snippet shows how to use Redis Graph with raw Redis commands from Python with [redis-py](https://github.com/andymccurdy/redis-py):

```python
import redis

r = redis.StrictRedis()
reply = r.execute_command('ML.FOREST.ADD', 'myforst', 0, '.', 'NUMERIC', 1, 0.1, '.l', 'LEAF', 1, '.r', 'LEAF', '0')
```

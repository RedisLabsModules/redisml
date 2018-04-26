# Machine Learning Model Server on Redis

Redis-ML is a Redis module that implements several machine learning models as Redis data types.

The stored models are fully operational and support the prediction/evaluation process.

Redis-ML is a turnkey solution for using trained models in a production environment. Load ML models from any platform, immediately ready to serve.

## Primary Features

* Decision Tree ensembles (random forests) classification and regression
* Linear regression
* Logistic regression
* Matrix operations

## Building and Running

1. Build a Redis server with support for modules (currently available from the [unstable branch](https://github.com/antirez/redis/tree/unstable)).

2. You'll also need a [BLAS](http://www.netlib.org/blas/) library such as [ATLAS](http://math-atlas.sourceforge.net/). To install ATLAS:

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

3. Build the Redis-ML module:

  ```sh
  git clone https://github.com/RedisLabsModules/redis-ml.git
  cd redis-ml/src
  make
  ```

4. To load the module, start Redis with the `--loadmodule /path/to/redis-ml/src/redis-ml.so` option, add it as a directive to the configuration file or send a `MODULE LOAD` command.

## Contributing

Issue reports, pull and feature requests are welcome.

## License

AGPLv3 - see [LICENSE](https://github.com/RedisLabsModules/redis-ml/blob/master/LICENSE)

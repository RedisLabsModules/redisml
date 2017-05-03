# Machine Learning Model Server on Redis

Redis-ML is a Redis module that implements several machine learning models as Redis data types.

The stored models are fully operational and support performing the prediction/evaluation.

Redis-ML is a turn key solution for using trained models in a production environment. Allowing loading ML models from any platform immediately ready to serve.

The module includes these primary features:

* Decision Tree ensembles (random forests) classification and regression
* Linear regression
* Logistic regression
* Matrix operations

## Building and running

- Build a Redis server with support for modules (currently available from the [unstable branch](https://github.com/antirez/redis/tree/unstable)).

- You'll also need a [BLAS](http://www.netlib.org/blas/) library, for example [ATLAS](http://math-atlas.sourceforge.net/). To install ATLAS:

  - Ubuntu: `sudo apt-get install libatlas-base-dev`
  - CentOS/RHEL/Fedora: `sudo yum install atlas-devel`

- Build the Redis-ML module:

  ```sh
  git clone https://github.com/RedisLabsModules/redis-ml.git
  cd redis-ml/src
  make
  ```

- To load the module, start Redis with the `--loadmodule /path/to/redis-ml/src/redis-ml.so` option, add it as a directive to the configuration file or send a `MODULE LOAD` command.

## Contributing

Issue reports, pull and feature requests are welcome.

## License

AGPLv3 - see [LICENSE](LICENSE)
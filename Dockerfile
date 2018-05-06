FROM ubuntu

RUN apt-get -y update && apt-get install -y build-essential git libatlas-base-dev
RUN git clone https://github.com/antirez/redis.git
RUN git clone https://github.com/RedisLabsModules/redis-ml.git
RUN cd redis && make && make install
RUN cd ..
RUN cd redis-ml/src && make

EXPOSE 6379
CMD ["redis-server", "--bind", "0.0.0.0", "--loadmodule", "/redis-ml/src/redis-ml.so"]


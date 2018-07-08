INDELUCES=./deps/tensorflow/include
DEPS=./deps/tensorflow/lib

all:
	$(MAKE) -C ./src all

test:
	$(MAKE) -C ./src $@

clean:
	$(MAKE) -C ./src $@

package: all
	$(MAKE) -C ./src package
.PHONY: package

deploydocs:
	mkdocs build
	s3cmd sync site/ s3://redisml.io
.PHONY: deploydocs

print_version:
	$(MAKE) -C ./src print_version

docker:
	docker build . -t redislabs/redisml

docker_push: docker
	docker push redislabs/redisml:latest

# ==================================================================================================================
#
# Docker to ubuntu 18.04 base image (for required libraries) used by GCBM
#
# Building this Docker:
#   docker build --build-arg NUM_CPU=14 -t moja/gcbm:ubuntu-18.04 .
#
# ==================================================================================================================

FROM moja/baseimage:ubuntu-18.04
ENV DEBIAN_FRONTEND=noninteractive

# set environment variables
ENV ROOTDIR /usr/local/
WORKDIR $ROOTDIR/

ENV ZIPPER_VERSION 1.0.1
ENV PATH $ROOTDIR/bin:$PATH
ENV LD_LIBRARY_PATH $ROOTDIR/lib:$ROOTDIR/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH
ENV PYTHONPATH $ROOTDIR/lib:$PYTHONPATH
ENV CURL_CA_BUNDLE /etc/ssl/certs/ca-certificates.crt
ENV GDAL_DATA=/usr/local/share/gdal
ENV GDAL_HTTP_VERSION 2
ENV GDAL_HTTP_MERGE_CONSECUTIVE_RANGES YES
ENV GDAL_HTTP_MULTIPLEX YES

# extra deps.
RUN apt-get update -y && apt-get install -y \
		doxygen \
		doxygen-latex \
		graphviz\
    postgis \
	&& 	apt-get clean

RUN	git clone --recursive https://github.com/sebastiandev/zipper.git \
	&& cd zipper && git checkout e9f150516cb55d194b5e01d21a9527783e98311d && mkdir build  && cd build \
	&& cmake .. \
	&& make --quiet -j $NUM_CPU \
  && make --quiet install \
  && make clean

# moja.global
RUN cd src && git clone -b develop https://github.com/moja-global/FLINT flint

# moja.canada
WORKDIR $ROOTDIR/src/flint/Source/build
RUN git clone -b develop https://github.com/moja-global/moja.canada

# build moja
WORKDIR $ROOTDIR/src/flint/Source/build
RUN cmake -DCMAKE_BUILD_TYPE=DEBUG  \
          -DCMAKE_INSTALL_PREFIX=$ROOTDIR \
          -DENABLE_TESTS:BOOL=OFF \
          -DENABLE_MOJA.MODULES.GDAL=ON \
          -DENABLE_MOJA.MODULES.LIBPQ=ON \
          -DBoost_USE_STATIC_LIBS=OFF \
          -DBUILD_SHARED_LIBS=ON .. \
	&& make --quiet -j $NUM_CPU \
	&& make --quiet install \
	&& make clean

WORKDIR $ROOTDIR/src
RUN ln -s /usr/local/lib/libmoja.modules.* /usr/local/bin
RUN rm -Rf /usr/local/src/*

WORKDIR $ROOTDIR/flint/Source/Build/bin
ENTRYPOINT ["./moja.cli"]

# ==================================================================================================================
#
# Docker to ubuntu 18.04 base image (for required libraries) used by GCBM
#
# Building this Docker:
#   docker build --build-arg NUM_CPU=4 -t moja/gcbm:ubuntu-18.04 .
#
# ==================================================================================================================

FROM ghcr.io/moja-global/flint.core:master AS base

ARG NUM_CPU=1
ARG BUILD_TYPE=DEBUG

ENV DEBIAN_FRONTEND=noninteractive
USER root

# set environment variables
ENV ROOTDIR /usr/local/
WORKDIR $ROOTDIR/

ENV PATH $ROOTDIR/bin:$PATH
ENV POCO_VERSION 1.9.2
ENV LD_LIBRARY_PATH $ROOTDIR/lib:$ROOTDIR/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH
ENV PYTHONPATH $ROOTDIR/lib:$PYTHONPATH
ENV CURL_CA_BUNDLE /etc/ssl/certs/ca-certificates.crt
ENV GDAL_DATA=/usr/local/share/gdal
ENV GDAL_HTTP_VERSION 2
ENV GDAL_HTTP_MERGE_CONSECUTIVE_RANGES YES
ENV GDAL_HTTP_MULTIPLEX YES

# Extra deps.
RUN apt-get update -y && apt-get install -y \
		doxygen \
		doxygen-latex \
		graphviz \
        unixodbc \
        unixodbc-dev \
        postgis \
	&& apt-get clean

# Rebuild POCO - ensure ODBC is included.
WORKDIR /tmp
RUN wget --progress=dot:giga https://pocoproject.org/releases/poco-${POCO_VERSION}/poco-${POCO_VERSION}-all.tar.gz \
    && tar -xzf poco-${POCO_VERSION}-all.tar.gz && mkdir poco-${POCO_VERSION}-all/cmake-build && cd poco-${POCO_VERSION}-all/cmake-build \
    && cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX=$ROOTDIR \
            -DPOCO_UNBUNDLED=ON \
            -DPOCO_STATIC=OFF \
            -DENABLE_ENCODINGS=OFF \
            -DENABLE_ENCODINGS_COMPILER=OFF \
            -DENABLE_XML=OFF \
            -DENABLE_JSON=ON \
            -DENABLE_MONGODB=OFF \
            -DENABLE_REDIS=OFF \
            -DENABLE_PDF=OFF \
            -DENABLE_UTIL=OFF \
            -DENABLE_NET=OFF \
            -DENABLE_NETSSL=OFF \
            -DENABLE_CRYPTO=OFF \
            -DENABLE_DATA=ON \
            -DENABLE_DATA_SQLITE=ON \
            -DENABLE_DATA_MYSQL=OFF \
            -DENABLE_DATA_ODBC=ON \
            -DENABLE_SEVENZIP=OFF \
            -DENABLE_ZIP=OFF \
            -DENABLE_PAGECOMPILER=OFF \
            -DENABLE_PAGECOMPILER_FILE2PAGE=OFF \
            -DENABLE_TESTS:BOOL=OFF .. \
	&& make --quiet -j $NUM_CPU \
    && make --quiet install/strip \
    && make clean \
    && cd $ROOTDIR/src

# moja.canada requires libpqxx.
WORKDIR /tmp
RUN git clone --recursive https://github.com/jtv/libpqxx.git \
    && cd libpqxx \
    && mkdir build \
    && cd build \
    && cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX=/usr/local/ \
        -DSKIP_BUILD_TEST=ON \
        -FSKIP_PQXX_STATIC=ON .. \
        -DCMAKE_CXX_FLAGS=-fPIC \
        -DPostgreSQL_TYPE_INCLUDE_DIR=/usr/include/postgresql/ \
    && make --quiet -j $NUM_CPU install/strip \
    && make clean

# moja.canada
# RUN cd $ROOTDIR/src && git clone -b develop https://github.com/moja-global/moja.canada

FROM base

WORKDIR $ROOTDIR/src

RUN mkdir -p moja.canada

COPY . ./moja.canada

WORKDIR $ROOTDIR/src/moja.canada/Source/build
RUN cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
           -DCMAKE_INSTALL_PREFIX=/usr/local \
           -DENABLE_TESTS:BOOL=OFF \
           -DPostgreSQL_TYPE_INCLUDE_DIR=/usr/include/postgresql/ \
           .. \
    && make --quiet -s -j $NUM_CPU \
    && make --quiet install \
    && make clean

WORKDIR $ROOTDIR/src
RUN mkdir -p /opt/gcbm
RUN ln -t /opt/gcbm /usr/local/lib/lib*
RUN ln -t /opt/gcbm /usr/local/bin/moja.cli
RUN rm -rf /usr/local/src/*
RUN rm -rf /tmp/*

#WORKDIR /opt/gcbm
#ENTRYPOINT ["./moja.cli"]

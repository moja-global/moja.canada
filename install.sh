#!/usr/bin/env bash

export NUM_CPU=1
export BUILD_TYPE=RELEASE
export DEBIAN_FRONTEND=noninteractive

# Setting Environment Variables

export ROOTDIR=/usr/local
export GDAL_VERSION=3.4.1
export POCO_VERSION=1.11.1
export BOOST_VERSION=1_78_0
export BOOST_VERSION_DOT=1.78.0
export FMT_VERSION=8.1.1

export PATH=$ROOTDIR/bin:$PATH
export LD_LIBRARY_PATH=$ROOTDIR/lib:$LD_LIBRARY_PATH
export PYTHONPATH=$ROOTDIR/lib:$PYTHONPATH

export CURL_CA_BUNDLE=/etc/ssl/certs/ca-certificates.crt
export GDAL_DATA=/usr/local/share/gdal
export GDAL_HTTP_VERSION=2
export GDAL_HTTP_VERSION=2
export GDAL_HTTP_MERGE_CONSECUTIVE_RANGES=YES
export GDAL_HTTP_MULTIPLEX=YES

export LC_ALL=C.UTF-8
export LANG=C.UTF-8

mkdir -p $ROOTDIR/ && cd $ROOTDIR/

# Install base dependencies

sudo apt-get update -y \
    && sudo apt-get install -y --fix-missing --no-install-recommends \
      software-properties-common build-essential ca-certificates \
      git g++ make cmake libssl-dev openssl wget bash-completion nasm \
      pkg-config libtool automake  libcurl4-gnutls-dev \
      zlib1g-dev libpcre3-dev libxml2-dev libexpat-dev libxerces-c-dev \
      doxygen doxygen-latex graphviz \
  	  python3-dev python3-numpy python3-pip \
      libproj-dev libgeos-dev \
      unixodbc unixodbc-dev \
      libspatialite-dev libsqlite3-dev sqlite3 \
      libpq-dev postgresql-client-12 postgresql-server-dev-12 postgis\
    && sudo apt-get -y autoremove \
  	&& sudo apt-get clean

sudo apt-get install -y

sudo mkdir -p $ROOTDIR/src && cd $ROOTDIR/src

# Installing POCO

sudo wget -q https://github.com/pocoproject/poco/archive/refs/tags/poco-${POCO_VERSION}-release.tar.gz \
    && sudo tar -xzf poco-${POCO_VERSION}-release.tar.gz \
    && sudo mkdir poco-poco-${POCO_VERSION}-release/cmake-build \
    && cd poco-poco-${POCO_VERSION}-release/cmake-build \
    && sudo cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
        -DCMAKE_INSTALL_PREFIX=$ROOTDIR \
        -DPOCO_UNBUNDLED=ON \
        -DENABLE_JSON=ON \
        -DENABLE_DATA=ON \
        -DENABLE_DATA_ODBC=ON \
        -DENABLE_DATA_SQLITE=ON \
        -DENABLE_DATA_MYSQL=OFF \
        -DENABLE_ACTIVERECORD=OFF \
        -DENABLE_ACTIVERECORD_COMPILER=OFF \
        -DENABLE_ENCODINGS=OFF \
        -DENABLE_ENCODINGS_COMPILER=OFF \
        -DENABLE_XML=OFF \
        -DENABLE_MONGODB=OFF \
        -DENABLE_REDIS=OFF \
        -DENABLE_PDF=OFF \
        -DENABLE_UTIL=OFF \
        -DENABLE_NET=OFF \
        -DENABLE_NETSSL=OFF \
        -DENABLE_CRYPTO=OFF \
        -DENABLE_SEVENZIP=OFF \
        -DENABLE_ZIP=OFF \
        -DENABLE_PAGECOMPILER=OFF \
        -DENABLE_PAGECOMPILER_FILE2PAGE=OFF .. \
    && sudo make -j 2 \
    && sudo make install/strip \
    && sudo make clean \
    && cd $ROOTDIR/src

echo "using python : 3.6 : /usr ;" > ~/user-config.jam

# Installing Boost C++ libraries

sudo wget -q https://boostorg.jfrog.io/artifactory/main/release/${BOOST_VERSION_DOT}/source/boost_${BOOST_VERSION}.tar.bz2 \
    && sudo tar --bzip2 -xf boost_${BOOST_VERSION}.tar.bz2 \
    && cd boost_${BOOST_VERSION}  \
    && sudo ./bootstrap.sh --prefix=$ROOTDIR \
    && sudo ./b2 -d0 -j $NUM_CPU cxxstd=14 install variant=release link=shared  \
    && sudo ./b2 clean \
    && cd $ROOTDIR/src

# Installing fmtlib

sudo wget -q https://github.com/fmtlib/fmt/archive/${FMT_VERSION}.tar.gz \
    && sudo mkdir libfmt-${FMT_VERSION} \
    && sudo tar -xzf ${FMT_VERSION}.tar.gz -C libfmt-${FMT_VERSION} --strip-components=1 \
    && cd libfmt-${FMT_VERSION} \
    && sudo cmake -G"Unix Makefiles" \
            -DCMAKE_BUILD_TYPE=RELEASE \
            -DCMAKE_INSTALL_PREFIX=$ROOTDIR \
            -DFMT_DOC=OFF \
            -DFMT_TEST=OFF . \
    && sudo make -j $NUM_CPU install/strip \
    && sudo make clean \
    && cd $ROOTDIR/src

# Installing GDAL

sudo wget -q http://download.osgeo.org/gdal/${GDAL_VERSION}/gdal-${GDAL_VERSION}.tar.gz \
    && sudo tar -xvf gdal-${GDAL_VERSION}.tar.gz && cd gdal-${GDAL_VERSION} \
    && sudo ./configure \
        --without-libtool \
        --with-hide-internal-symbols \
        --with-python \
        --with-spatialite \
        --with-pg \
        --with-curl \
        --prefix=$ROOTDIR \
        --with-libtiff=internal \
        --with-rename-internal-libtiff-symbols \
        --with-geotiff=internal \
        --with-rename-internal-libgeotiff-symbols \
    && sudo make -j 4 \
    && sudo make install \
    && sudo make clean \
    && cd $ROOTDIR/src

sudo strip -s $ROOTDIR/lib/libgdal.so
# sudo for i in $ROOTDIR/lib/python3/dist-packages/osgeo/*.so; do strip -s $i 2>/dev/null || /bin/true; done

sudo strip -s $ROOTDIR/bin/gdal_contour \
    && sudo strip -s $ROOTDIR/bin/gdal_grid \
    && sudo strip -s $ROOTDIR/bin/gdal_rasterize \
    && sudo strip -s $ROOTDIR/bin/gdal_translate \
    && sudo strip -s $ROOTDIR/bin/gdaladdo \
    && sudo strip -s $ROOTDIR/bin/gdalbuildvrt \
    && sudo strip -s $ROOTDIR/bin/gdaldem \
    && sudo strip -s $ROOTDIR/bin/gdalenhance \
    && sudo strip -s $ROOTDIR/bin/gdalinfo \
    && sudo strip -s $ROOTDIR/bin/gdallocationinfo \
    && sudo strip -s $ROOTDIR/bin/gdalmanage \
    && sudo strip -s $ROOTDIR/bin/gdalsrsinfo \
    && sudo strip -s $ROOTDIR/bin/gdaltindex \
    && sudo strip -s $ROOTDIR/bin/gdaltransform \
    && sudo strip -s $ROOTDIR/bin/gdalwarp \
    && sudo strip -s $ROOTDIR/bin/gnmanalyse \
    && sudo strip -s $ROOTDIR/bin/gnmmanage \
    && sudo strip -s $ROOTDIR/bin/nearblack \
    && sudo strip -s $ROOTDIR/bin/ogr2ogr \
    && sudo strip -s $ROOTDIR/bin/ogrinfo \
    && sudo strip -s $ROOTDIR/bin/ogrlineref \
    && sudo strip -s $ROOTDIR/bin/ogrtindex

sudo apt-get update -y \
    && sudo apt-get remove -y --purge build-essential \
    && cd $ROOTDIR/src/gdal-${GDAL_VERSION}/swig/python \
    && sudo python3 setup.py build \
    && sudo python3 setup.py install

# Installing Zipper

sudo git clone --recursive https://github.com/sebastiandev/zipper.git \
    && sudo mkdir zipper/build \
    && cd zipper/build \
  	&& sudo cmake .. \
  	&& sudo make -j 4 \
    && sudo make install \
    && sudo make clean \
    && cd $ROOTDIR/src

sudo ldconfig
sudo rm -r $ROOTDIR/src/*

sudo mkdir -p $ROOTDIR/src && cd $ROOTDIR/src

# Cloning and building the FLINT

sudo git clone -b update-poco https://github.com/moja-global/FLINT.git flint \
    && sudo mkdir -p flint/Source/build \
    && cd flint/Source/build \
    && sudo cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE  \
            -DCMAKE_INSTALL_PREFIX=$ROOTDIR \
            -DENABLE_TESTS:BOOL=OFF \
            -DENABLE_MOJA.MODULES.GDAL=ON \
            -DENABLE_MOJA.MODULES.ZIPPER=ON \
            -DENABLE_MOJA.MODULES.LIBPQ=ON \
            -DBoost_USE_STATIC_LIBS=OFF \
            -DBUILD_SHARED_LIBS=ON .. \
  	&& sudo make -j 4 \
  	&& sudo make install \
    && cd $ROOTDIR/src

sudo ln -s $ROOTDIR/lib/libmoja.modules.* $ROOTDIR/bin
sudo rm -Rf $ROOTDIR/src/* \
    && sudo ldconfig

cd $ROOTDIR/src

sudo git clone --recursive https://github.com/jtv/libpqxx.git \
    && sudo mkdir libpqxx/build \
    && cd libpqxx/build \
    && sudo cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
            -DCMAKE_INSTALL_PREFIX=/usr/local \
            -DSKIP_BUILD_TEST=ON \
            -FSKIP_PQXX_STATIC=ON .. \
            -DCMAKE_CXX_FLAGS=-fPIC \
            -DPostgreSQL_TYPE_INCLUDE_DIR=/usr/include/postgresql \
    && sudo make -j $NUM_CPU install/strip \
    && sudo make clean \
    && cd $ROOTDIR/src

# Cloning and building the GCBM

sudo git clone -b develop https://github.com/moja-global/moja.canada \
    && sudo mkdir -p moja.canada/Source/build \
    && cd moja.canada/Source/build \
    && sudo cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
            -DCMAKE_INSTALL_PREFIX=$ROOTDIR \
            -DENABLE_TESTS:BOOL=OFF \
            -DPostgreSQL_TYPE_INCLUDE_DIR=/usr/include/postgresql .. \
    && sudo make -s -j $NUM_CPU \
    && sudo make install \
    && sudo make clean \
    && cd $ROOTDIR/src

sudo mkdir -p /opt/gcbm
sudo ln -t /opt/gcbm /usr/local/lib/lib*
sudo ln -t /opt/gcbm /usr/local/bin/moja.cli

# Running the GCBM

/opt/gcbm/moja.cli --version

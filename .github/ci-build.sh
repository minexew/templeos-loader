#!/bin/sh
set -ex


git clone git://git.musl-libc.org/musl musl-src
cd musl-src
./configure --prefix=$PWD/../build/musl --disable-shared
make install
cd ..
export PATH=$PWD/build/musl/bin:$PATH

git submodule update --init --recursive
cd physfslt-3.0.2
env CC=musl-gcc cmake -DCMAKE_INSTALL_PREFIX:PATH=$PWD/../build/physfs -DPHYSFS_BUILD_SHARED=OFF . && make install
cd ..

mkdir cmake-build-debug
cd cmake-build-debug
env CC=musl-gcc PHYSFSDIR=$PWD/../build/physfs/ cmake ..
cmake --build . --target templeos-loader

#!/bin/sh
set -ex

git clone git://git.musl-libc.org/musl musl
cd musl
mkdir build
./configure --prefix=$PWD/build --disable-shared
make install
export PATH=$PWD/build/bin:$PATH
echo $PATH
cd ..

curl -L -O https://icculus.org/physfs/downloads/physfs-3.0.2.tar.bz2
tar xfa physfs-3.0.2.tar.bz2
cd physfs-3.0.2
mkdir build
env CC=musl-gcc cmake -DCMAKE_INSTALL_PREFIX:PATH=build -DPHYSFS_BUILD_SHARED=OFF . && make install
export PHYSFSDIR=$PWD/build
cd ..

mkdir build
cd build
env CC=musl-gcc cmake ..
cmake --build .

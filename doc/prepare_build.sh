#!/bin/bash

INSTALL_DIR="/opt"

# install dependencies
apt-get install -y gcc g++ make build-essential git wget unzip tar curl \
  valgrind clang-tidy python3.7 python3-pip

add-apt-repository ppa:ethereum/ethereum
apt-get update
apt-get install solc

if ! command -v cmake; then
  # install cmake
  cd ${INSTALL_DIR}
  CMAKE_TARGET_VERSION=3.15.5
  wget https://github.com/Kitware/CMake/releases/download/v${CMAKE_TARGET_VERSION}/cmake-${CMAKE_TARGET_VERSION}.tar.gz
  tar -xvf cmake-${CMAKE_TARGET_VERSION}.tar.gz
  rm cmake-${CMAKE_TARGET_VERSION}.tar.gz
  cd cmake-${CMAKE_TARGET_VERSION}
  ./bootstrap
  make -j$(nproc)
  make install
  cmake --version || exit 1
fi

pip3 install conan

conan remote remove conan-center || exit 1

conan remote add heshu http://conan.heshu:9300 False || exit 1

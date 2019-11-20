#!/usr/bin/env bash

GRPC_BRANCH="master"
GRPC_URL="https://github.com/grpc/grpc"
GRPC_SOURCE_DIR="/var/local/git/grpc"
PROCESSORS="$(nproc)"

apt-get update
apt-get install -y git curl build-essential autoconf pkg-config automake libtool make g++ unzip zlib1g-dev

git clone -b ${GRPC_BRANCH} ${GRPC_URL} ${GRPC_SOURCE_DIR}
cd ${GRPC_SOURCE_DIR} || exit 1
git submodule update --init

echo "--- installing protobuf ---"
cd ${GRPC_SOURCE_DIR}/third_party/protobuf || exit 1
./autogen.sh
./configure --enable-shared
make -j${PROCESSORS}
make -j${PROCESSORS} check
make install
make clean
ldconfig

echo "--- installing grpc ---"
cd ${GRPC_SOURCE_DIR} || exit 1
make CFLAGS='-g -O2 -w' CXXFLAGS='-g -O2 -w' -j${PROCESSORS}
make install
make clean
ldconfig

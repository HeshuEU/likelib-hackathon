#!/bin/bash

FOLDER=${PWD}
cd /opt || exit 1
git clone --recurse-submodules https://github.com/Microsoft/vcpkg.git
cd ./vcpkg || exit 1
apt-get install curl unzip tar g++
./bootstrap-vcpkg.sh
./vcpkg install boost
./vcpkg install grpc
./vcpkg install openssl
cd "${FOLDER}" || exit 1

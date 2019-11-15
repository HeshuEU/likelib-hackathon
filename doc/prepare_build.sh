#!/bin/bash

cd ~
git clone --recurse-submodules https://github.com/Microsoft/vcpkg.git
cd ./vcpkg/
./bootstrap-vcpkg.sh
sudo apt-get install curl unzip tar
./bootstrap-vcpkg.sh
./vcpkg install boost
./vcpkg install grpc
./vcpkg install openssl


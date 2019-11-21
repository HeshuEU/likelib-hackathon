#!/bin/bash

INSTALL_DIR="/opt"

# install dependencies
apt-get install -y gcc g++ make build-essential wget git unzip tar curl

# install cmake
CMAKE_VERSION=3.16.0-rc4
cd ${INSTALL_DIR} || exit 1
wget "https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-Linux-x86_64.sh"
CMAKE_EXECUTE_FILE="cmake-${CMAKE_VERSION}-Linux-x86_64.sh"
chmod +x ${CMAKE_EXECUTE_FILE}
./${CMAKE_EXECUTE_FILE} --skip-license --prefix=${INSTALL_DIR}
rm ${CMAKE_EXECUTE_FILE}
${INSTALL_DIR}/bin/cmake --version || exit 1

# install vcpkg
if [ -d "${INSTALL_DIR}/vcpkg" ]; then
  echo "You have vcpkg in ${INSTALL_DIR}"
    cd "${INSTALL_DIR}/vcpkg" || exit 1

else
  cd ${INSTALL_DIR} || exit 1
  git clone --recurse-submodules https://github.com/Microsoft/vcpkg.git  || exit 1
  cd vcpkg || exit 1
  ./bootstrap-vcpkg.sh --disableMetrics
fi

# install packages by vcpkg
./vcpkg install openssl
./vcpkg install boost
./vcpkg install grpc
./vcpkg install leveldb

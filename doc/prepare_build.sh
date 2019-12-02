#!/bin/bash

INSTALL_DIR="/opt"

#read -p "Enter install path [${INSTALL_DIR}]:"
#NEW_INSTALL_PATH=$REPLY
#mkdir -p ${NEW_INSTALL_PATH}

#if [ -w $NEW_INSTALL_PATH ]; then
#  echo "NEW install path ${NEW_INSTALL_PATH}"
#  INSTALL_DIR=${NEW_INSTALL_PATH}
#else
#  echo "Can't write to ${NEW_INSTALL_PATH}"
#  echo "Exit..."
#  exit 1
#fi

SCRIPT_DIR=${PWD}
if [ ! -f "${SCRIPT_DIR}/prepare_build.sh" ]; then
  echo "Run script from doc folder"
  exit 1
fi

# install dependencies
apt-get install -y gcc g++ make build-essential wget git unzip tar curl

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

# install vcpkg
if [ -d "${INSTALL_DIR}/vcpkg" ]; then
  echo "You have vcpkg in ${INSTALL_DIR}"
  cd "${INSTALL_DIR}/vcpkg" || exit 1
else
  cd ${INSTALL_DIR} || exit 1
  git clone --recurse-submodules https://github.com/Microsoft/vcpkg.git || exit 1
  cd vcpkg || exit 1
  ./bootstrap-vcpkg.sh --disableMetrics
fi

# install packages by vcpkg
./vcpkg install openssl
./vcpkg install boost
./vcpkg install grpc
./vcpkg install leveldb

chown 1000:1000 -R ../vcpkg
echo "runing change bachrc script"

PATH_TO_BASH_RC="/home/${SUDO_USER}/.bashrc"

if [[ "${EUID}" -ne 0 ]]; then
  PATH_TO_BASH_RC="/home/${USER}/.bashrc"
fi

if cat ${PATH_TO_BASH_RC} | grep "#===========likelib============="; then
  echo "You have likelib $(cat ${PATH_TO_BASH_RC} | grep likelib_SOURCE_DIR)"
  echo "Exit..."
  exit
else
  echo "Start changing ${PATH_TO_BASH_RC} for user:\"$SUDO_USER\""

  cd "${SCRIPT_DIR}/../" || exit 1

  echo "#===========likelib=============" >>"${PATH_TO_BASH_RC}"
  echo -e "zmake () {\n
    SOURCE_DIR=${PWD}\n
    if [[ -f ./CMakeLists.txt ]]; then\n
      SOURCE_DIR=\${PWD}\n
    elif [[ -f ../CMakeLists.txt ]]; then\n
      SOURCE_DIR=\${PWD}\/..\n
    fi\n
    echo Build to \${PWD}\n
    echo From \${SOURCE_DIR}\n
    cmake -DCMAKE_TOOLCHAIN_FILE=${INSTALL_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake -S \${SOURCE_DIR} -B \${PWD} \n
    }" >>"${PATH_TO_BASH_RC}"

fi

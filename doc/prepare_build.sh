#!/bin/bash

if apt-get update > /dev/null; then
   SUDO_PREF=""
else
   SUDO_PREF=sudo
fi

# Update for install apt-utils
${SUDO_PREF} apt-get update || exit 1
${SUDO_PREF} apt-get install apt-utils software-properties-common -y || exit 1
# Add all repository
${SUDO_PREF} add-apt-repository ppa:ethereum/ethereum -y || exit 1

${SUDO_PREF} apt-get update || exit 1
# If we download new ubuntu image we don't need dist-upgrage, but on user machine it's work
# Maby dist-upgrade can broke somebody on the user machine
${SUDO_PREF} apt-get dist-upgrade -y || exit 1

# Install other software
${SUDO_PREF} apt-get install -y git wget unzip tar curl valgrind \
                                clang-tidy python3.7 python3-pip \
                                solc autoconf libtool python3-dev || exit 1
pip3 install web3 || exit 1
pip3 install coincurve || exit 1

#cmake --version || (${SUDO_PREF} apt-get install -y build-essential gcc g++ make cmake || exit 1)
if ! command -v cmake; then
  # install cmake
  DIR_PATH=$PWD
  ${SUDO_PREF} apt-get install -y build-essential gcc g++ make libssl-dev
  ${SUDO_PREF} mkdir /opt/cmake && ${SUDO_PREF} chown $UID /opt/cmake
  cd /opt/cmake
  CMAKE_TARGET_VERSION=3.16.5
  wget https://github.com/Kitware/CMake/releases/download/v${CMAKE_TARGET_VERSION}/cmake-${CMAKE_TARGET_VERSION}.tar.gz || exit 1
  tar -xvf cmake-${CMAKE_TARGET_VERSION}.tar.gz || exit 1
  rm cmake-${CMAKE_TARGET_VERSION}.tar.gz || exit 1
  cd cmake-${CMAKE_TARGET_VERSION}
  ./bootstrap || exit 1
  make -j$(nproc) || exit 1
  ${SUDO_PREF} make install || exit 1
  cd $DIR_PATH
  rm -rf /opt/cmake
  cmake --version || exit 1
fi

# Install conan
if ! command -v conan; then
  ${SUDO_PREF} pip3 install conan || exit 1
  conan remote list | grep -q conan-center && (conan remote remove conan-center || exit 1)
  conan remote list | grep -q heshu || (conan remote add heshu https://conan.heshu.by || exit 1)
fi
if ! g++-9 --version; then
  echo -e "g++-9 \e[31mnot found\e[0m! Check you OS (need Ubuntu 19.10), or install g++-9"
  echo -e "Installation  - \e[31m FAILED \e[0m"
  exit 1
fi

echo
echo -e "Installation  - \e[32m SUCCESS \e[0m"
echo "Now you can compile project"

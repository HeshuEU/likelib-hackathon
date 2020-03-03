#!/bin/bash

if apt-get update > /dev/null; then
   SUDO_PREF=""
else
   SUDO_PREF=sudo
fi

${SUDO_PREF} apt-get update || exit 1
${SUDO_PREF} apt-get install apt-utils -y || exit 1
${SUDO_PREF} apt-get update || exit 1
${SUDO_PREF} apt-get dist-upgrade -y || exit 1
${SUDO_PREF} apt-get update || exit 1

${SUDO_PREF} apt-get install -y build-essential gcc g++ make cmake || exit 1

${SUDO_PREF} apt-get install -y git wget unzip tar curl valgrind clang-tidy python3.7 python3-pip || exit 1

${SUDO_PREF} apt-get install -y  software-properties-common || exit 1
${SUDO_PREF} apt-get update || exit 1

${SUDO_PREF} add-apt-repository ppa:ethereum/ethereum -y || exit 1
${SUDO_PREF} apt-get update || exit 1
${SUDO_PREF} apt-get install -y solc || exit 1

${SUDO_PREF} apt-get install -y autoconf libtool || exit 1

pip3 install conan || exit 1
conan remote remove conan-center || exit 1
conan remote add heshu https://conan.heshu.by || exit 1

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

cmake --version || (${SUDO_PREF} apt-get install -y build-essential gcc g++ make cmake || exit 1)

${SUDO_PREF} apt-get install -y git wget unzip tar curl valgrind \
                                clang-tidy python3.7 python3-pip \
                                solc autoconf libtool || exit 1

# Installed with apt-utils
#${SUDO_PREF} apt-get install -y  software-properties-common || exit 1
#${SUDO_PREF} apt-get update || exit 1

# This repository added in other section update no need
#${SUDO_PREF} add-apt-repository ppa:ethereum/ethereum -y || exit 1
#${SUDO_PREF} apt-get update || exit 1
# Installed with other software, together with next line
#${SUDO_PREF} apt-get install -y solc || exit 1

#${SUDO_PREF} apt-get install -y autoconf libtool || exit 1

pip3 install conan || exit 1
conan remote list | grep -q conan-center && (conan remote remove conan-center || exit 1)
conan remote list | grep -q heshu || (conan remote add heshu https://conan.heshu.by || exit 1)

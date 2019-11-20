#!/bin/bash

## Default parametrs
INSTALL_DIR="/home/user"
TARGET_VCPKG_FOLDER="vcpkg"
SCRIPT_DIR=${PWD}
CHANGE_BASHRC=true
INSTALL_SOFTWARE=true
INSTALL_VCPKG=true
INSTALL_FOR_DOCKER=false

## Check parametrs
while [[ -n "${1}" ]]; do
  case ${1} in
  "--without-bashrc")
    CHANGE_BASHRC=false
    ;;
  "--without-software")
    INSTALL_SOFTWARE=false
    ;;
  "--without-vcpkg")
    INSTALL_VCPKG=false
    ;;
  "--install-for-docker")
    INSTALL_FOR_DOCKER=true
    ;;
  "-h" | "--help")
    echo "Usage: ./prepare_build.sh [options]"
    echo
    echo "Options:"
    echo "    -h, --help              Show help oprions"
    echo "    --install-for-docker    Use this option for install as root"
    echo "                            ATANTION if you work as user you need install as user"
    echo "                            а ещё тут не помешает пояснение нормальное"
    echo "    -min, --without-bashrc  Don't change user setting (you can't use zbuild and zmake)"
    echo "    --without-install-software   Don't install software"
    echo "    --without-check-software  Don't check software g++-9 ..."
    exit 0
    ;;

  *)
    echo "Unknown argument ${1}. Use '--help' for help."
    exit 1
    ;;
  esac

  shift

done

if ! [ -f "${SCRIPT_DIR}/prepare_build.sh" ]; then
    echo "Please run script from his dir"
    exit 1
fi
if [ $UID = 0 -a $INSTALL_FOR_DOCKER != true ]; then
    echo "Run scritp as non root user or with option --install-for-docker"
    exit 1
fi
if $INSTALL_FOR_DOCKER; then
    echo "Install for docker"
    INSTALL_DIR="/"
    CHANGE_BASHRC=false
    INSTALL_SOFTWARE=false
else
    read -p "Enter install path [$INSTALL_DIR]:"
    NEW_INSTALL_PATH=$REPLY
fi
if [ -n "$NEW_INSTALL_PATH" ]; then
  echo "NEW path $NEW_INSTALL_PATH"
  mkdir -p $NEW_INSTALL_PATH || (echo "Can't create $NEW_INSTALL_PATH" && exit 1)
else
  echo "OLD path $INSTALL_DIR"
fi

## run workflow
## run install software
if "${INSTALL_SOFTWARE}"; then
  echo "runing install software script"
  if ! command -v g++; then
    echo "Please, for continue run"
    echo "sudo apt install -y g++"
    exit 1
  fi
  if ! command -v make; then
    echo "Please, for continue run"
    echo "sudo apt install -y make"
    exit 1
  fi
  if ! command -v git; then
    echo "Please, for continue run"
    echo "sudo apt install -y git"
    exit 1
  fi
  if ! command -v curl; then
    echo "Please, for continue run"
    echo "sudo apt install -y curl"
    exit 1
  fi
  if ! command -v unzip; then
    echo "Please, for continue run"
    echo "sudo apt install -y unzip"
    exit 1
  fi
  if ! command -v tar; then
    echo "Please, for continue run"
    echo "sudo apt install -y tar"
    exit 1
  fi
  if ! command -v wget; then
    echo "Please, for continue run"
    echo "sudo apt install -y wget"
    exit 1
  fi
  if ! command -v cmake; then
    echo "Try to cd ${INSTALL_DIR}"
    cd ${INSTALL_DIR} || exit 1

    CMAKE_TARGET_VERSION=3.15.5
#    wget "https://github.com/Kitware/CMake/releases/download/v${CMAKE_TARGET_VERSION}/cmake-${CMAKE_TARGET_VERSION}.tar.gz"  || exit 1
#    tar -xvf "cmake-${CMAKE_TARGET_VERSION}.tar.gz"  || exit 1
#    rm "cmake-${CMAKE_TARGET_VERSION}.tar.gz"
#    echo "Try to cd cmake-${CMAKE_TARGET_VERSION}"
#    cd "cmake-${CMAKE_TARGET_VERSION}" || exit 1
#    echo "Try to build cmake"

#    ./bootstrap || exit 1
#    make -j ${PROCESSORS} || exit 1
#    sudo make install || exit 1

    cd /tmp
    wget https://github.com/Kitware/CMake/releases/download/v3.16.0-rc4/cmake-3.16.0-rc4-Linux-x86_64.sh
    . ./cmake-3.16.0-rc4-Linux-x86_64.sh --skip-license --prefix=$INSTALL_DIR
    rm ./cmake-3.16.0-rc4-Linux-x86_64.sh
    $INSTALL_DIR/bin/cmake --version || exit 1
  fi

fi


if "${INSTALL_VCPKG}"; then
  echo "runing install vcpkg script"

  if [ -d "${INSTALL_DIR}/${TARGET_VCPKG_FOLDER}" ]; then
    echo "You have vcpkg in ${INSTALL_DIR}"
    echo "Exit..."
    exit 1
  fi

  git clone --recurse-submodules https://github.com/Microsoft/vcpkg.git "${INSTALL_DIR}/${TARGET_VCPKG_FOLDER}"  || exit 1
  echo "Try to cd ${INSTALL_DIR}/${TARGET_VCPKG_FOLDER}"
  cd "${INSTALL_DIR}/${TARGET_VCPKG_FOLDER}" || exit 1

  ./bootstrap-vcpkg.sh --disableMetrics

  ./vcpkg install openssl
  ./vcpkg install boost
  ./vcpkg install grpc
fi

if "${CHANGE_BASHRC}"; then
  echo "runing change bachrc script"

  PATH_TO_BASH_RC="/home/${SUDO_USER}/.bashrc"

  if [[ "${EUID}" -ne 0 ]]; then
    PATH_TO_BASH_RC="/home/${USER}/.bashrc"
  fi

  if cat ${PATH_TO_BASH_RC} | grep likelib_SOURCE_DIR ; then
    echo "You have likelib $(cat ${PATH_TO_BASH_RC} | grep likelib_SOURCE_DIR)"
    echo "Exit..."
    exit
  else
    echo "Start changing ${PATH_TO_BASH_RC} for user:\"$SUDO_USER\""

    cd "${SCRIPT_DIR}/../" || exit 1

    echo "#===========likelib=============" >> "${PATH_TO_BASH_RC}"
    echo "export likelib_SOURCE_DIR=${PWD}" >> "${PATH_TO_BASH_RC}"
    echo -e "zmake () {\n
    $INSTALL_DIR/bin/cmake \${likelib_SOURCE_DIR} -DCMAKE_TOOLCHAIN_FILE=${INSTALL_DIR}/${TARGET_VCPKG_FOLDER}/scripts/buildsystems/vcpkg.cmake \n
    }" >> "${PATH_TO_BASH_RC}"

  fi

fi


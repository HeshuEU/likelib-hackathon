#!/bin/bash

mkdir -p ../build
cd ../build
cmake .. "-DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake"
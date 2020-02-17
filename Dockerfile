FROM ubuntu:19.10

ENV SOURCE_DIR /sources/project/
COPY . ${SOURCE_DIR}

RUN ${SOURCE_DIR}/doc/prepare_build.sh

ENV BUILD_DIR /builds/release/project/
RUN mkdir -p ${BUILD_DIR}

RUN cmake -DCMAKE_BUILD_TYPE=Release -S ${SOURCE_DIR} -B ${BUILD_DIR}

RUN cd ${BUILD_DIR} && make -j$(nproc)

RUN cd ${BUILD_DIR}/bin && \
    ./run_tests --log_level=test_suite --detect_memory_leaks=1 --build_info


# INSTALL_DIR="/opt"
#    - SOURCE_DIR=${PWD}
#    - BUILD_DIR="${PWD}/build"
#    - mkdir -p ${BUILD_DIR}
#    - mkdir -p /tmp/logs
#    - cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_CLANG_TIDY="${SOURCE_DIR}/cmake/clang-tidy.py;-checks=*;-header-filter=.*;" -S ${SOURCE_DIR} -B ${BUILD_DIR}
#    - cd ${BUILD_DIR} && make -j$(nproc)

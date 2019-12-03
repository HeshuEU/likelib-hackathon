FROM ubuntu:19.10 as build

# update system
RUN apt-get update && \
    apt-get install -y software-properties-common && \
    apt-get update -y && \
    apt-get install -y apt-utils && \
    apt-get dist-upgrade -y

ENV INSTALL_DIR /opt

WORKDIR ${INSTALL_DIR}

COPY doc/prepare_build.sh ${INSTALL_DIR}

RUN ./prepare_build.sh

# build project
ENV PROJECT_SOURCE_DIR /project
ENV PROJECT_BUILD_DIR /build

COPY . ${PROJECT_SOURCE_DIR}

WORKDIR ${PROJECT_BUILD_DIR}

RUN cmake ${PROJECT_SOURCE_DIR} -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake && \
    make -j$(nproc)

# run unit tests
RUN ${PROJECT_BUILD_DIR}/test/run_tests --log_level=test_suite --detect_memory_leaks=1

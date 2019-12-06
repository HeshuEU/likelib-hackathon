# Create docker environment for build project
FROM ubuntu:19.10 as build

# update system
RUN apt-get update && \
    apt-get install -y software-properties-common && \
    apt-get update -y && \
    apt-get install -y apt-utils && \
    apt-get dist-upgrade -y

# set work dir to /opt
ENV INSTALL_DIR /opt
WORKDIR ${INSTALL_DIR}

# set up environment for building project
COPY doc/prepare_build.sh ${INSTALL_DIR}
RUN ./prepare_build.sh

# build project
ENV PROJECT_SOURCE_DIR /project
ENV PROJECT_BUILD_DIR /build
COPY . ${PROJECT_SOURCE_DIR}
WORKDIR ${PROJECT_BUILD_DIR}
RUN cmake ${PROJECT_SOURCE_DIR} -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake && \
    make -j$(nproc)

RUN ls / && ls /opt

# Create docker environment for run test without any dependents
FROM ubuntu:19.10 as test

WORKDIR /likelib

# copy test executable
COPY --from=build /build/test/run_tests .

# run unit tests
RUN ./run_tests --log_level=test_suite --detect_memory_leaks=1 --build_info

RUN ls / && ls /opt /likelib -l

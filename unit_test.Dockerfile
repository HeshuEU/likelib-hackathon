# Create docker environment for run test without any dependents
FROM likelib2_build as build
FROM ubuntu:19.10 as test

WORKDIR /likelib

# copy test executable
COPY --from=build /build/test/unit_test/run_tests .

# run unit tests
RUN ./run_tests --log_level=test_suite --detect_memory_leaks=1 --build_info

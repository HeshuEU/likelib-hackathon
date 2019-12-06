# Create docker environment for run test without any dependents
FROM likelib2_build as code_quality

WORKDIR /likelib

# copy test executable
COPY --from=code_quality /build/test/run_tests .

# run unit tests
RUN ./run_tests --log_level=test_suite --detect_memory_leaks=1 --build_info

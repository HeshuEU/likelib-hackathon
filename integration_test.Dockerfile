# Create docker environment for run test without any dependents
FROM likelib2_build as build
FROM ubuntu:19.10 as test

# update system
RUN apt-get update && \
    apt-get install -y software-properties-common && \
    apt-get update -y && \
    apt-get install -y apt-utils && \
    apt-get dist-upgrade -y

RUN apt-get install python3.7

ENV TEST_FILES_DIR /likelib
WORKDIR ${TEST_FILES_DIR}

# copy node executable
COPY --from=build /build/src/node/node .

# copy rpc-client executable
COPY --from=build /build/src/rpc-client/rpc-client .

# copy test scripts
COPY --from=build /project/test/integration_test test_scripts

# run integration tests
RUN python3 test_scripts/run_test.py -n ${TEST_FILES_DIR}/node -r ${TEST_FILES_DIR}/rpc-client


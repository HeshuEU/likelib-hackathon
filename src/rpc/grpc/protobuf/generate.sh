#!/usr/bin/env bash

# C++ generate

INPUT_FILE="public_rpc.proto"
OUTPUT_DIR="../gen"
PLUGIN_PATH="/usr/local/bin/grpc_cpp_plugin"

protoc --grpc_out="${OUTPUT_DIR}" --plugin=protoc-gen-grpc="${PLUGIN_PATH}" "${INPUT_FILE}"
protoc --cpp_out="${OUTPUT_DIR}" "${INPUT_FILE}"

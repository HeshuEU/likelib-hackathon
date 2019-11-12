#!/usr/bin/env bash

INPUT_FILE="public_rpc.proto"
OUTPUT_DIR="../src/rpc/impl/gen"
PLUGIN_PATH="/usr/local/bin/grpc_cpp_plugin"

protoc --grpc_out="${OUTPUT_DIR}" --plugin=protoc-gen-grpc="${PLUGIN_PATH}" "${INPUT_FILE}"
protoc --cpp_out="${OUTPUT_DIR}" "${INPUT_FILE}"

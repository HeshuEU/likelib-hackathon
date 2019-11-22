#!/usr/bin/env bash

PROTOBUF_FILE=public_rpc.proto
PROTO_PATH=../../src/rpc/grpc/protobuf/

python -m grpc_tools.protoc --python_out=. --grpc_python_out=. --proto_path=${PROTO_PATH} "${PROTO_PATH}/${PROTOBUF_FILE}"

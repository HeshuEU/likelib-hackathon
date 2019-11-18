find_package(Protobuf REQUIRED)
find_package(GRPC REQUIRED)
find_package(ZLIB REQUIRED)

set(PROTOBUF_FILE_NAME public_rpc.proto)
set(LOCAL_PROTOBUF_DIR grpc/protobuf)
set(LOCAL_GENERATE_OUTPUT_DIR grpc/gen)
set(GLOBAL_PROTOBUF_FILE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${LOCAL_PROTOBUF_DIR}/${PROTOBUF_FILE_NAME})
set(GLOBAL_OUTPUT_CODE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${LOCAL_GENERATE_OUTPUT_DIR})
set(GLOBAL_PROTOBUF_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${LOCAL_PROTOBUF_DIR})

execute_process(COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} --proto_path=${GLOBAL_PROTOBUF_DIR} --grpc_out=${GLOBAL_OUTPUT_CODE_DIR} --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN} ${GLOBAL_PROTOBUF_FILE_PATH})
execute_process(COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} --proto_path=${GLOBAL_PROTOBUF_DIR} --cpp_out=${GLOBAL_OUTPUT_CODE_DIR} "${GLOBAL_PROTOBUF_FILE_PATH}")

set(GRPC_GENERATED
        ${LOCAL_GENERATE_OUTPUT_DIR}/public_rpc.pb.cc
        ${LOCAL_GENERATE_OUTPUT_DIR}/public_rpc.pb.h
        ${LOCAL_GENERATE_OUTPUT_DIR}/public_rpc.grpc.pb.cc
        ${LOCAL_GENERATE_OUTPUT_DIR}/public_rpc.grpc.pb.h
        )

set(RPC_HEADERS
        grpc/grpc_service.hpp
        grpc/grpc_node_server.hpp
        grpc/grpc_node_client.hpp
        error.hpp
        rpc.hpp
        )

set(RPC_SOURCES
        grpc/grpc_service.cpp
        grpc/grpc_node_server.cpp
        grpc/grpc_node_client.cpp
        )

add_library(rpc ${RPC_HEADERS} ${RPC_SOURCES} ${GRPC_GENERATED})

target_link_libraries(rpc base bc gRPC::grpc++ gRPC::grpc protobuf::libprotobuf ZLIB::ZLIB)

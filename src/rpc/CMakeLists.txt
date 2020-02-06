set(GLOBAL_PROTOBUF_FILE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/grpc/protobuf/public_rpc.proto)
set(GLOBAL_OUTPUT_CODE_DIR ${CMAKE_BINARY_DIR}/grpc/gen)

GRPC_GENERATE_CPP(GENERATED_SOURCES GENERATED_HEADERS ${GLOBAL_OUTPUT_CODE_DIR} ${GLOBAL_PROTOBUF_FILE_PATH})

set(RPC_HEADERS
        grpc/grpc_adapter.hpp
        grpc/grpc_node_server.hpp
        grpc/grpc_node_client.hpp
        base_rpc.hpp
        error.hpp
        rpc.hpp
        )

set(RPC_SOURCES
        grpc/grpc_adapter.cpp
        grpc/grpc_node_server.cpp
        grpc/grpc_node_client.cpp
        )

add_library(rpc ${RPC_HEADERS} ${RPC_SOURCES} ${GENERATED_HEADERS} ${GENERATED_SOURCES})

target_include_directories(rpc PUBLIC ${GLOBAL_OUTPUT_CODE_DIR} ${GRPC_INCLUDE_DIR} ${PROTOBUF_INCLUDE_DIR})

target_link_libraries(rpc base bc gRPC::grpc++_unsecure gRPC::grpc++_error_details gRPC::grpc_cronet gRPC::address_sorting gRPC::grpc++_reflection gRPC::grpc_unsecure gRPC::grpcpp_channelz gRPC::gpr gRPC::grpc++ gRPC::grpc gRPC::upb protobuf::libprotobuf cares::cares zlib::zlib)

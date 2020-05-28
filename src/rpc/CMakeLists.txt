set(GLOBAL_PROTOBUF_FILE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/grpc/protobuf/public_rpc.proto)
set(GLOBAL_OUTPUT_CODE_DIR ${CMAKE_BINARY_DIR}/grpc/gen)

GRPC_GENERATE_CPP(GENERATED_SOURCES GENERATED_HEADERS ${GLOBAL_OUTPUT_CODE_DIR} ${GLOBAL_PROTOBUF_FILE_PATH})

set(RPC_HEADERS
        grpc/grpc_adapter.hpp
        grpc/grpc_server.hpp
        grpc/grpc_client.hpp
        grpc/tools.hpp
        http/http_adapter.hpp
        http/http_client.hpp
        http/http_server.hpp
        http/tools.hpp
        base_rpc.hpp
        error.hpp
        rpc.hpp
        )

set(RPC_SOURCES
        grpc/grpc_adapter.cpp
        grpc/grpc_server.cpp
        grpc/grpc_client.cpp
        grpc/tools.cpp
        http/http_adapter.cpp
        http/http_client.cpp
        http/http_server.cpp
        http/tools.cpp
        rpc.cpp)

include_directories(SYSTEM ${GLOBAL_OUTPUT_CODE_DIR})

add_library(rpc ${RPC_HEADERS} ${RPC_SOURCES} ${GENERATED_HEADERS} ${GENERATED_SOURCES})

target_include_directories(rpc SYSTEM PUBLIC ${GLOBAL_OUTPUT_CODE_DIR})
target_include_directories(rpc PUBLIC ${RPC_MODULE_INCLUDE})

target_link_libraries(rpc base core gRPC::grpc_module cpprestsdk::cpprest OpenSSL::SSL pthread Boost::system dl crypto)

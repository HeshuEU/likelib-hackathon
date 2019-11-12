find_package(Protobuf REQUIRED)
find_package(GRPC REQUIRED)
find_package(ZLIB REQUIRED)

set(GENERATED
        impl/gen/public_rpc.pb.cc
        impl/gen/public_rpc.pb.h
        impl/gen/public_rpc.grpc.pb.cc
        impl/gen/public_rpc.grpc.pb.h
        impl/hello_world_server.cpp)

set(RPC_HEADERS
        base_server.hpp
        base_client.hpp
        )

set(RPC_SOURCES
        base_server.cpp
        base_client.cpp

        )

add_library(rpc ${RPC_HEADERS} ${RPC_SOURCES})
add_executable(rpc-test ${GENERATED})

target_link_libraries(rpc gRPC::grpc++ gRPC::grpc protobuf::libprotobuf ZLIB::ZLIB)
target_link_libraries(rpc-test gRPC::grpc++ gRPC::grpc protobuf::libprotobuf ZLIB::ZLIB)

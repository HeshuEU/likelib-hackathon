find_package(Protobuf REQUIRED)
find_package(GRPC REQUIRED)
find_package(ZLIB REQUIRED)

set(GENERATED
        impl/grpc/gen/public_rpc.pb.cc
        impl/grpc/gen/public_rpc.pb.h
        impl/grpc/gen/public_rpc.grpc.pb.cc
        impl/grpc/gen/public_rpc.grpc.pb.h
        impl/grpc/hello_world_server.cpp)

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

target_link_libraries(rpc-test base gRPC::grpc++ gRPC::grpc protobuf::libprotobuf ZLIB::ZLIB)

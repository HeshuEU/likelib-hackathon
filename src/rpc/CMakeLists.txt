find_package(Protobuf REQUIRED)
find_package(GRPC REQUIRED)
find_package(ZLIB REQUIRED)

set(GENERATED
        grpc/gen/public_rpc.pb.cc
        grpc/gen/public_rpc.pb.h
        grpc/gen/public_rpc.grpc.pb.cc
        grpc/gen/public_rpc.grpc.pb.h
        )

set(RPC_HEADERS
        grpc/grpc_service.hpp
        grpc/grpc_node_server.hpp
        grpc/grpc_node_client.hpp
        rpc.hpp
        )

set(RPC_SOURCES
        grpc/grpc_service.cpp
        grpc/grpc_node_server.cpp
        grpc/grpc_node_client.cpp
        )

add_library(rpc ${RPC_HEADERS} ${RPC_SOURCES} ${GENERATED})

target_link_libraries(rpc base bc gRPC::grpc++ gRPC::grpc protobuf::libprotobuf ZLIB::ZLIB)

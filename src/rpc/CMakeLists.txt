set(RPC_HEADERS
        base_rpc_server.hpp
        base_rpc_client.hpp
        )

set(RPC_SOURCES
        base_rpc_server.cpp
        base_rpc_client.cpp
        )

add_library(rpc ${RPC_HEADERS} ${RPC_SOURCES})
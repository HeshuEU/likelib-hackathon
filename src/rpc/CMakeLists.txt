set(RPC_HEADERS
        base_server.hpp
        base_client.hpp
        )

set(RPC_SOURCES
        base_server.cpp
        base_client.cpp
        )

add_library(rpc ${RPC_HEADERS} ${RPC_SOURCES})
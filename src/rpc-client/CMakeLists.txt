set(RPC_CLIENT_HEADERS
        )

set(RPC_CLIENT_SOURCES
        main.cpp
        )

add_executable(rpc-client ${RPC_CLIENT_HEADERS} ${RPC_CLIENT_SOURCES})

target_link_libraries(rpc-client rpc)

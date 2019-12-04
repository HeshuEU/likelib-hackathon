set(RPC_CLIENT_HEADERS
        parameters_helper.hpp
        parameters_helper.tpp
        )

set(RPC_CLIENT_SOURCES
        parameters_helper.cpp
        main.cpp
        )

add_executable(rpc-client ${RPC_CLIENT_HEADERS} ${RPC_CLIENT_SOURCES})

target_link_libraries(rpc-client rpc base)

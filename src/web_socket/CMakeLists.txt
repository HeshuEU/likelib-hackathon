set(RPC_HEADERS
        tools.hpp
        session.hpp
        server.hpp
        client.hpp
        )

set(RPC_SOURCES
        tools.cpp
        session.cpp
        server.cpp
        client.cpp
        )

add_library(rpc ${RPC_HEADERS} ${RPC_SOURCES})

target_link_libraries(rpc base core Boost::thread)

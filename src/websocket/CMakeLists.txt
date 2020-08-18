set(RPC_HEADERS
        tools.hpp
        connection.hpp
        session.hpp
        acceptor.hpp
        client.hpp
        )

set(RPC_SOURCES
        tools.cpp
        connection.cpp
        session.cpp
        acceptor.cpp
        client.cpp
        )

add_library(websocket ${RPC_HEADERS} ${RPC_SOURCES})

target_link_libraries(websocket base core Boost::thread)

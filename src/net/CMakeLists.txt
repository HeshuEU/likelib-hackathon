set(NET_HEADERS
        acceptor.hpp
        connection.hpp
        connector.hpp
        error.hpp
        host.hpp
        endpoint.hpp
        packet.hpp
        peer.hpp
        session.hpp
        )

set(NET_SOURCES
        acceptor.cpp
        connection.cpp
        connector.cpp
        host.cpp
        endpoint.cpp
        packet.cpp
        peer.cpp
        session.cpp
        )

add_library(net ${NET_SOURCES} ${NET_HEADERS})

target_link_libraries(net base bc)

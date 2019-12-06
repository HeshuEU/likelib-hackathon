set(NET_HEADERS
        connection.hpp
        error.hpp
        host.hpp
        endpoint.hpp
        packet.hpp
        peer.hpp
        )

set(NET_SOURCES
        connection.cpp
        host.cpp
        endpoint.cpp
        packet.cpp
        peer.cpp
        )

add_library(net ${NET_SOURCES} ${NET_HEADERS})

target_link_libraries(net base bc Boost::serialization)

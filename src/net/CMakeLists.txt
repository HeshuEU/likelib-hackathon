set(NET_HEADERS
        connection.hpp
        error.hpp
        network.hpp
        endpoint.hpp
        packet.hpp
        )

set(NET_SOURCES
        connection.cpp
        network.cpp
        endpoint.cpp
        packet.cpp
        )

add_library(net ${NET_SOURCES} ${NET_HEADERS})

target_link_libraries(net base Boost::serialization)

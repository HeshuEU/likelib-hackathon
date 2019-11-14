set(NET_HEADERS
        connection.hpp
        error.hpp
        manager.hpp
        endpoint.hpp
        packet.hpp
        )

set(NET_SOURCES
        connection.cpp
        manager.cpp
        endpoint.cpp
        packet.cpp
        )

add_library(net ${NET_SOURCES} ${NET_HEADERS})

target_link_libraries(net base Boost::serialization)

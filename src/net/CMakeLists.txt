set(NET_HEADERS
        connection.hpp
        error.hpp
        manager.hpp
        network_address.hpp
        packet.hpp
        )

set(NET_SOURCES
        connection.cpp
        manager.cpp
        network_address.cpp
        packet.cpp
        )

add_library(net ${NET_SOURCES} ${NET_HEADERS})

target_link_libraries(net base Boost::serialization)

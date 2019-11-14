set(NETWORK_HEADERS
        connection.hpp
        error.hpp
        manager.hpp
        network_address.hpp
        packet.hpp
        )

set(NETWORK_SOURCES
        connection.cpp
        manager.cpp
        network_address.cpp
        packet.cpp
        )

add_library(network ${NETWORK_SOURCES} ${NETWORK_HEADERS})

target_link_libraries(network base Boost::serialization)

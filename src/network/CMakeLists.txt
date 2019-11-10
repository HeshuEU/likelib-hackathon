set(NETWORK_HEADERS
        connection_acceptor.hpp
        connection.hpp
        manager.hpp
        network_address.hpp
        )

set(NETWORK_SOURCES
        connection_acceptor.cpp
        connection.cpp
        manager.cpp
        network_address.cpp
        )

add_library(network ${NETWORK_SOURCES} ${NETWORK_HEADERS})

target_link_libraries(network base)

set(NET_HEADERS
        acceptor.hpp
        connection.hpp
        connector.hpp
        error.hpp
        endpoint.hpp
        session.hpp
        )

set(NET_SOURCES
        acceptor.cpp
        connection.cpp
        connector.cpp
        endpoint.cpp
        session.cpp
        )

add_library(net ${NET_SOURCES} ${NET_HEADERS})

target_link_libraries(net base core)

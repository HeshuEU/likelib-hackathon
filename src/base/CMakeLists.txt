set(BASE_HEADERS
        assert.hpp
        error.hpp
        stringifiable_enum_class.hpp
        assert.hpp
        config.hpp
        log.hpp
        types.hpp
        address.hpp
        filesystem/directory.hpp
        property_tree.hpp
        property_tree.tpp
        network/connection_acceptor.hpp
        network/connection.hpp
        network/manager.hpp
        bytes.hpp
        network/ipv4_address.hpp
        )

set(BASE_SOURCES
        error.cpp
        log.cpp
        address.cpp
        filesystem/directory.cpp
        property_tree.cpp
        network/connection_acceptor.cpp
        network/connection.cpp
        network/manager.cpp
        bytes.cpp
        network/ipv4_address.cpp
        )

add_library(base ${BASE_SOURCES} ${BASE_HEADERS})

target_link_libraries(base Boost::log dl backtrace)

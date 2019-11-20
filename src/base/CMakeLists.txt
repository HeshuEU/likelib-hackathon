set(BASE_HEADERS
        assert.hpp
        big_integer.hpp
        big_integer.tpp
        error.hpp
        stringifiable_enum_class.hpp
        assert.hpp
        config.hpp
        log.hpp
        types.hpp
        directory.hpp
        property_tree.hpp
        property_tree.tpp
        bytes.hpp
        hash.hpp
        timer.hpp
        serialization.hpp
        serialization.tpp
        )

set(BASE_SOURCES
        error.cpp
        log.cpp
        directory.cpp
        property_tree.cpp
        bytes.cpp
        hash.cpp
        timer.cpp
        serialization.cpp
        )

add_library(base ${BASE_SOURCES} ${BASE_HEADERS})

target_link_libraries(base Boost::log dl backtrace)

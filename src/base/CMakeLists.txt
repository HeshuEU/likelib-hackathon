set(BASE_HEADERS
        assert.hpp
        error.hpp
        stringifiable_enum_class.hpp
        assert.hpp
        config.hpp
        log.hpp
        types.hpp
        filesystem/directory.hpp
        property_tree.hpp
        property_tree.tpp
        bytes.hpp
        )

set(BASE_SOURCES
        error.cpp
        log.cpp
        filesystem/directory.cpp
        property_tree.cpp
        bytes.cpp
        )

add_library(base ${BASE_SOURCES} ${BASE_HEADERS})

target_link_libraries(base Boost::log dl backtrace)

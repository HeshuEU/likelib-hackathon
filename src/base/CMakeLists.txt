set(BASE_HEADERS
        assert.hpp
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
        program_options.hpp
        )

set(BASE_SOURCES
        error.cpp
        log.cpp
        directory.cpp
        property_tree.cpp
        bytes.cpp
        hash.cpp
        program_options.cpp
        )

add_library(base ${BASE_SOURCES} ${BASE_HEADERS})

target_link_libraries(base Boost::log Boost::program_options dl backtrace)

set(BASE_HEADERS
        assert.hpp
        big_num.hpp
        big_num.tpp
        error.hpp
        stringifiable_enum_class.hpp
        status.hpp
        status.tpp
        result.hpp
        result.tpp
        assert.hpp
        config.hpp
        log.hpp
        types.hpp
        address.hpp
        filesystem/directory.hpp
        )

set(BASE_SOURCES
        error.cpp
        status.cpp
        result.cpp
        log.cpp
        address.cpp
        filesystem/directory.cpp
        )

add_library(base ${BASE_SOURCES} ${BASE_HEADERS})

target_link_libraries(base Boost::log)

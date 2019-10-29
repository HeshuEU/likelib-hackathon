set(BASE_HEADERS
        error.hpp
        stringifiable_enum_class.hpp
        status.hpp
        status.tpp
        result.hpp
        result.tpp
        assert.hpp config.hpp)

set(BASE_SOURCES
        error.cpp
        status.cpp
        result.cpp
)

add_library(base ${BASE_SOURCES} ${BASE_HEADERS})

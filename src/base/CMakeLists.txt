set(BASE_TEMPLATES
        hash.tpp
        database.tpp
        property_tree.tpp
        serialization.tpp
        program_options.tpp)

set(BASE_HEADERS
        assert.hpp
        crypto.hpp
        big_integer.hpp
        error.hpp
        utility.hpp
        config.hpp
        log.hpp
        types.hpp
        directory.hpp
        property_tree.hpp
        bytes.hpp
        hash.hpp
        program_options.hpp
        database.hpp
        serialization.hpp
        time.hpp
        )

set(BASE_SOURCES
        config.cpp
        crypto.cpp
        error.cpp
        log.cpp
        directory.cpp
        property_tree.cpp
        bytes.cpp
        hash.cpp
        program_options.cpp
        database.cpp
        serialization.cpp
        time.cpp
        )

add_library(base ${BASE_HEADERS} ${BASE_TEMPLATES} ${BASE_SOURCES})
target_link_libraries(base Boost::log Boost::log_setup Boost::program_options OpenSSL::SSL dl backtrace leveldb::leveldb secp256k1_lib)

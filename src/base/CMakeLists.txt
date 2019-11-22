set(BASE_HEADERS
        assert.hpp
		crypto.hpp
		asymmetric_crypto.hpp
		symmetric_crypto.hpp
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
        program_options.hpp
        program_options.tpp
        timer.hpp
        database.hpp
        )

set(BASE_SOURCES
        crypto.cpp
        asymmetric_crypto.cpp
        symmetric_crypto.cpp
        error.cpp
        log.cpp
        directory.cpp
        property_tree.cpp
        bytes.cpp
        hash.cpp
        timer.cpp
        program_options.cpp
        database.cpp
        )

add_library(base ${BASE_SOURCES} ${BASE_HEADERS})

target_link_libraries(base Boost::log Boost::program_options OpenSSL::SSL dl backtrace leveldb::leveldb)

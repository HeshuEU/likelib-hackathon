set(BASE_HEADERS
        assert.hpp
		crypto.hpp
        big_integer.hpp
        big_integer.tpp
        error.hpp
        utility.hpp
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
        serialization.hpp
        serialization.tpp
        time.hpp
        subprogram_router.hpp
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
        timer.cpp
        program_options.cpp
        database.cpp
        serialization.cpp
        time.cpp
        subprogram_router.cpp
        )

add_library(base ${BASE_SOURCES} ${BASE_HEADERS})
target_link_libraries(base Boost::log Boost::log_setup Boost::program_options OpenSSL::SSL dl backtrace leveldb::leveldb)

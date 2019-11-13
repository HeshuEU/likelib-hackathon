set(TEST_SOURCES
        main.cpp
        base/big_integer.cpp
        base/log.cpp
        base/property_tree.cpp
        base/bytes.cpp
        base/hash.cpp
        network/packet.cpp
        rpc/base_rpc.cpp
        )

add_executable(run_tests ${TEST_SOURCES})

target_link_libraries(run_tests base network rpc Boost::unit_test_framework)

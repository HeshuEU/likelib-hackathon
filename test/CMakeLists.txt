set(TEST_SOURCES
        main.cpp
        base/log.cpp
        base/property_tree.cpp
        rpc/base_rpc.cpp
        )

add_executable(run_tests ${TEST_SOURCES})

target_link_libraries(run_tests base rpc Boost::unit_test_framework)

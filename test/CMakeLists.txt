set(TEST_SOURCES
        main.cpp
        base/error_codes.cpp
        base/status.cpp
        base/result.cpp
        base/log.cpp
        base/plain_result_use.cpp
        rpc/base_rpc.cpp
        )

add_executable(run_tests ${TEST_SOURCES})

target_link_libraries(run_tests base rpc Boost::unit_test_framework)

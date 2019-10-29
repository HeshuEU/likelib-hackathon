set(TEST_SOURCES main.cpp ./base/error_codes.cpp base/status.cpp base/result.cpp)

add_executable(run_tests ${TEST_SOURCES})
target_link_libraries(run_tests base Boost::unit_test_framework)

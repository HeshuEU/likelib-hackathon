set(TEST_SOURCES
        main.cpp
        base/big_integer.cpp
        base/bytes.cpp
        base/crypto.cpp
        base/database.cpp
        base/error.cpp
        base/hash.cpp
        base/program_options.cpp
        base/property_tree.cpp
        base/serialization.cpp
        base/time.cpp
        base/timer.cpp
        bc/address.cpp
        bc/block.cpp
        bc/transaction.cpp
        bc/transactions_set.cpp
        net/endpoint.cpp
        vm/vm.cpp
        vm/tools.cpp
        )

add_executable(run_tests ${TEST_SOURCES})

target_link_libraries(run_tests base net rpc vm lk Boost::unit_test_framework dl)

target_compile_definitions(run_tests PRIVATE BOOST_STACKTRACE_USE_ADDR2LINE)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_link_options(run_tests PRIVATE "-no-pie")
endif ()
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
        net/packet.cpp
        bc/address.cpp
        bc/balance_manager.cpp
		bc/transaction.cpp
        bc/transactions_set.cpp
		)

add_executable(run_tests ${TEST_SOURCES})

target_link_libraries(run_tests base net rpc Boost::unit_test_framework)

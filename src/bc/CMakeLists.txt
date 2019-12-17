set(BC_HEADERS
        address.hpp
        block.hpp
        blockchain.hpp
        transaction.hpp
        types.hpp
        transactions_set.hpp
        database_manager.hpp
        )

set(BC_SOURCES
        address.cpp
        block.cpp
        blockchain.cpp
        transaction.cpp
        transactions_set.cpp
        database_manager.cpp
        )

add_library(bc ${BC_SOURCES} ${BC_HEADERS})

target_link_libraries(bc base)

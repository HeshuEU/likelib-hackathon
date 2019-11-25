set(BC_HEADERS
        address.hpp
        block.hpp
        blockchain.hpp
        miner.hpp
        transaction.hpp
        types.hpp
        balance_manager.hpp
        )

set(BC_SOURCES
        address.cpp
        block.cpp
        blockchain.cpp
        miner.cpp
        transaction.cpp
        balance_manager.cpp
        )

add_library(bc ${BC_SOURCES} ${BC_HEADERS})

target_link_libraries(bc base)

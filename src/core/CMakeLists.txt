set(LK_HEADERS
        address.hpp
        block.hpp
        blockchain.hpp
        core.hpp
        eth_adapter.hpp
        managers.hpp
        peer.hpp
        protocol.hpp
        transaction.hpp
        types.hpp
        transactions_set.hpp
        )

set(LK_SOURCES
        address.cpp
        block.cpp
        blockchain.cpp
        core.cpp
        eth_adapter.cpp
        managers.cpp
        peer.cpp
        protocol.cpp
        transaction.cpp
        transactions_set.cpp
        )

add_library(lk ${LK_SOURCES} ${LK_HEADERS})

target_link_libraries(lk base bc net vm)

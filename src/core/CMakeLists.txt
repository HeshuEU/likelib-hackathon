set(CORE_HEADERS
        address.hpp
        block.hpp
        blockchain.hpp
        consensus.hpp
        core.hpp
        host.hpp
        managers.hpp
        peer.hpp
        rating.hpp
        transaction.hpp
        types.hpp
        transactions_set.hpp
        )

set(CORE_TEMPLATES
        block.tpp
        )

set(CORE_SOURCES
        address.cpp
        block.cpp
        blockchain.cpp
        consensus.cpp
        core.cpp
        host.cpp
        managers.cpp
        messages.cpp
        peer.cpp
        rating.cpp
        transaction.cpp
        transactions_set.cpp
        )

add_library(core ${CORE_SOURCES} ${CORE_TEMPLATES} ${CORE_HEADERS})

target_link_libraries(core base net vm)

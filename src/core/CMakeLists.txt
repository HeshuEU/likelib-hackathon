set(CORE_HEADERS
        address.hpp
        block.hpp
        blockchain.hpp
        core.hpp
        host.hpp
        managers.hpp
        peer.hpp
        protocol.hpp
        transaction.hpp
        types.hpp
        transactions_set.hpp
        )

set(CORE_SOURCES
        address.cpp
        block.cpp
        blockchain.cpp
        core.cpp
        host.cpp
        managers.cpp
        peer.cpp
        protocol.cpp
        transaction.cpp
        transactions_set.cpp
        )

add_library(core ${CORE_SOURCES} ${CORE_HEADERS})

target_link_libraries(core base net vm)

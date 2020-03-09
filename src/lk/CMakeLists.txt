set(LK_HEADERS
        eth_adapter.hpp
        managers.hpp
        core.hpp
        protocol.hpp
        )

set(LK_SOURCES
        eth_adapter.cpp
        managers.cpp
        core.cpp
        protocol.cpp
        )

add_library(lk ${LK_SOURCES} ${LK_HEADERS})

target_link_libraries(lk base bc net vm)

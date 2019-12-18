set(LK_HEADERS
        balance_manager.hpp
        core.hpp
        protocol.hpp
        )

set(LK_SOURCES
        balance_manager.cpp
        core.cpp
        protocol.cpp
        )

add_library(lk ${LK_SOURCES} ${LK_HEADERS})

target_link_libraries(lk base bc net)

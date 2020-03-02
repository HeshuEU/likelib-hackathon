set(LK_HEADERS
        account_manager.hpp
        core.hpp
        protocol.hpp
        )

set(LK_SOURCES
        account_manager.cpp
        core.cpp
        protocol.cpp
        )

add_library(lk ${LK_SOURCES} ${LK_HEADERS})

target_link_libraries(lk base bc net vm)

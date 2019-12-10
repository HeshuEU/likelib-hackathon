set(LKCORE_HEADERS
        balance_manager.hpp
        core.hpp
        protocol.hpp
        )

set(LKCORE_SOURCES
        balance_manager.cpp
        core.cpp
        protocol.cpp
        )

add_library(lkcore ${LKCORE_SOURCES} ${LKCORE_HEADERS})

target_link_libraries(lkcore base bc net)

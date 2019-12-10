set(LKCORE_HEADERS
        lkcore.hpp
        )

set(LKCORE_SOURCES
        lkcore.cpp
        )

add_library(lkcore ${LKCORE_SOURCES} ${LKCORE_HEADERS})

target_link_libraries(lkcore base bc net)

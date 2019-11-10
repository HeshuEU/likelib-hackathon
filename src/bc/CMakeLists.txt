set(BC_HEADERS
        block.hpp
        )

set(BC_SOURCES
        block.cpp
        )

add_library(bc ${BC_SOURCES} ${BC_HEADERS})

target_link_libraries(bc base)

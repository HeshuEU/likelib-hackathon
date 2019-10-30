set(NODE_HEADERS
)

set(NODE_SOURCES
        main.cpp
)

add_executable(node ${NODE_SOURCES} ${NODE_HEADERS})

target_link_libraries(node base)

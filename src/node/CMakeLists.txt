set(NODE_HEADERS
        hard_config.hpp
        soft_config.hpp
        rpc_service.hpp
        )

set(NODE_SOURCES
        soft_config.cpp
        hard_config.cpp
        rpc_service.cpp
        main.cpp
        )

add_executable(node ${NODE_SOURCES} ${NODE_HEADERS})

target_link_libraries(node base net bc rpc)

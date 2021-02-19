set(NODE_HEADERS
        hard_config.hpp
        public_service.hpp
        miner.hpp
        node.hpp
        )

set(NODE_SOURCES
        hard_config.cpp
        public_service.cpp
        miner.cpp
        node.cpp
        main.cpp
        )

add_executable(node ${NODE_SOURCES} ${NODE_HEADERS})

target_link_libraries(node base core net websocket dl backtrace)

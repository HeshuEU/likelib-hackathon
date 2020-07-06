set(NODE_HEADERS
        hard_config.hpp
        soft_config.hpp
        public_service.hpp
        miner.hpp
        node.hpp
        )

set(NODE_SOURCES
        soft_config.cpp
        hard_config.cpp
        public_service.cpp
        miner.cpp
        node.cpp
        main.cpp
        )

add_executable(node ${NODE_SOURCES} ${NODE_HEADERS})

target_link_libraries(node base core net websocket dl backtrace)

target_compile_definitions(node PRIVATE BOOST_STACKTRACE_USE_ADDR2LINE)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_link_options(node PRIVATE "-no-pie")
endif ()

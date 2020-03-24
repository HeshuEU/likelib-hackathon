set(CLIENT_HEADERS
        actions.hpp
        config.hpp
        utility.hpp
        utility.tpp
        )

set(CLIENT_SOURCES
        actions.cpp
        main.cpp
        utility.cpp
        )

add_executable(client ${CLIENT_HEADERS} ${CLIENT_SOURCES})

target_link_libraries(client base core rpc vm)

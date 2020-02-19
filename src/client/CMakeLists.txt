set(CLIENT_HEADERS
        parameters_helper.hpp
        parameters_helper.tpp
        )

set(CLIENT_SOURCES
        parameters_helper.cpp
        main.cpp
        )

add_executable(client ${CLIENT_HEADERS} ${CLIENT_SOURCES})

target_link_libraries(client base rpc)

set(CLIENT_HEADERS
        subprogram_router.hpp
        actions.hpp
        config.hpp
        utility.hpp
        utility.tpp
        )

set(CLIENT_SOURCES
        subprogram_router.cpp
        actions.cpp
        main.cpp
        utility.cpp
        )

add_executable(client ${CLIENT_HEADERS} ${CLIENT_SOURCES})

target_link_libraries(client base core rpc vm)

target_compile_definitions(client PRIVATE BOOST_STACKTRACE_USE_ADDR2LINE)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_options(client PRIVATE "-no-pie")
endif ()

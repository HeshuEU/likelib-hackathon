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

target_link_libraries(client base rpc vm)

target_compile_definitions(run_tests PRIVATE BOOST_STACKTRACE_USE_ADDR2LINE)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_link_options(run_tests PRIVATE "-no-pie")
endif ()
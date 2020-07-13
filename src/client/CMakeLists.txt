# adding Cli(Header only) to project
add_library(cli INTERFACE)
add_library(cli::cli ALIAS cli)
target_include_directories(cli
        INTERFACE
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/cli/include>
        )

target_link_libraries(cli INTERFACE Boost::system Threads::Threads)
target_compile_features(cli INTERFACE cxx_std_20)
target_compile_definitions(cli INTERFACE BOOST_ASIO_NO_DEPRECATED=1)

set(CLIENT_HEADERS
        actions.hpp
        config.hpp
        utility.hpp
        utility.tpp
        client.hpp
        )

set(CLIENT_SOURCES
        actions.cpp
        main.cpp
        utility.cpp
        client.cpp
        )

add_executable(client ${CLIENT_HEADERS} ${CLIENT_SOURCES})

target_link_libraries(client base core websocket vm cli::cli)

target_compile_definitions(client PRIVATE BOOST_STACKTRACE_USE_ADDR2LINE)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_options(client PRIVATE "-no-pie")
endif ()

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
        client.hpp
        )

set(CLIENT_SOURCES
        actions.cpp
        main.cpp
        client.cpp
        )

find_library(READLINE_LIBRARY readline REQUIRED)
find_path(READLINE_INCLUDE_DIR readline/readline.h)
find_library(TERMCAP_LIBRARY termcap REQUIRED)

add_executable(client ${CLIENT_HEADERS} ${CLIENT_SOURCES})

target_link_libraries(client base core websocket vm cli::cli ${READLINE_LIBRARY} ${TERMCAP_LIBRARY})
target_include_directories(client SYSTEM PRIVATE ${READLINE_INCLUDE_DIR})
target_compile_definitions(client PRIVATE -DUSE_READLINE=1)

target_compile_definitions(client PRIVATE BOOST_STACKTRACE_USE_ADDR2LINE)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_options(client PRIVATE "-no-pie")
endif ()

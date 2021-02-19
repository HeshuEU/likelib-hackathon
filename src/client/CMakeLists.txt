
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

target_link_libraries(client base core websocket vm ${READLINE_LIBRARY} ${TERMCAP_LIBRARY})
target_include_directories(client SYSTEM PRIVATE ${READLINE_INCLUDE_DIR})
target_compile_definitions(client PRIVATE -DUSE_READLINE=1)

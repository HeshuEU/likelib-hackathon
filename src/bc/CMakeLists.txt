set(BC_HEADERS
        address.hpp
        block.hpp
        transaction.hpp
        types.hpp
        base_client.hpp
        base_service.hpp
        general_server_service.hpp
        )

set(BC_SOURCES
        address.cpp
        block.cpp
        transaction.cpp
        general_server_service.cpp
        )

add_library(bc ${BC_SOURCES} ${BC_HEADERS})

target_link_libraries(bc base)

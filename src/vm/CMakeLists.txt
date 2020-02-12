add_subdirectory(evmc)

set(VM_HEADERS
        error.hpp
        vm.hpp
        messages.hpp
        tools.hpp
        )

set(VM_SOURCES
        vm.cpp
        messages.cpp
        tools.cpp
        )


add_library(vm STATIC ${VM_HEADERS} ${VM_SOURCES})
set_target_properties(instructions PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${EVMC_INCLUDE_DIR}
        INTERFACE_LINK_DEPENDS "loader"
        )

target_include_directories(vm PUBLIC
        $<BUILD_INTERFACE:${EVMC_INCLUDE_DIR}>$<INSTALL_INTERFACE:include>
        )

target_link_libraries(vm base loader)

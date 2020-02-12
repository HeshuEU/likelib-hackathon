add_subdirectory(evmc)

set(VM_HEADERS
        vm.hpp
        solc.hpp
        )

set(VM_SOURCES
        vm.cpp
        solc.cpp
        )



add_library(vm STATIC ${VM_HEADERS} ${VM_SOURCES})
set_target_properties(instructions PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${EVMC_INCLUDE_DIR}
        INTERFACE_LINK_DEPENDS "loader"
        )



#target_include_directories(vm INTERFACE ${EVMC_INCLUDE_DIR})
target_include_directories(vm PUBLIC
        $<BUILD_INTERFACE:${EVMC_INCLUDE_DIR}>$<INSTALL_INTERFACE:include>
        )

#include_directories(${EVMC_INCLUDE_DIR})
target_link_libraries(vm base loader)

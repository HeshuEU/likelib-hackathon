add_subdirectory(evmc)

set(VM_HEADERS
        vm.hpp
        )

set(VM_SOURCES
        vm.cpp
        )

add_library(vm ${VM_HEADERS} ${VM_SOURCES})

target_link_libraries(vm base loader)

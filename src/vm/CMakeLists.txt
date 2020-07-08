add_subdirectory(evmc)

set(VM_HEADERS
        error.hpp
	encode_decode.hpp
        vm.hpp
        tools.hpp
        )

set(VM_SOURCES
	encode_decode.cpp
        vm.cpp
        tools.cpp
        )

set(EVMC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/evmc/include)

find_package(PythonLibs REQUIRED)

add_library(vm STATIC ${VM_HEADERS} ${VM_SOURCES})
target_include_directories(vm PUBLIC $<BUILD_INTERFACE:${EVMC_INCLUDE_DIR}>$<INSTALL_INTERFACE:include> ${PYTHON_INCLUDE_DIRS})
target_link_libraries(vm loader OpenSSL::SSL Boost::serialization python3.7m)

# copy evm libs
file(GLOB EVM_LIB ${CONAN_BIN_DIRS_EVMONE}/*evmone*)
file(COPY ${EVM_LIB} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/)

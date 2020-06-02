add_subdirectory(evmc)

set(VM_HEADERS
        error.hpp
        vm.hpp
        tools.hpp
        )

set(VM_SOURCES
        vm.cpp
        tools.cpp
        )

set(EVMC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/evmc/include)

FIND_PACKAGE(PythonLibs 3.7 REQUIRED)

add_library(vm STATIC ${VM_HEADERS} ${VM_SOURCES})
target_include_directories(vm PUBLIC $<BUILD_INTERFACE:${EVMC_INCLUDE_DIR}>$<INSTALL_INTERFACE:include> ${PYTHON_INCLUDE_DIRS})
target_link_libraries(vm loader OpenSSL::SSL Boost::serialization ${PYTHON_LIBRARIES})

# copy evm libs
file(GLOB EVM_LIB ${CONAN_BIN_DIRS_EVMONE}/*evmone*)
file(COPY ${EVM_LIB} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/)

# copy python files
file(GLOB PYTHON_FILES ${CMAKE_CURRENT_LIST_DIR}/python/*.py)
file(COPY ${PYTHON_FILES} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/)

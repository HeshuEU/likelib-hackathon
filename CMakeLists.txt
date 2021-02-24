cmake_minimum_required(VERSION 3.13)

project(likelib VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-redundant-move -pedantic -D_FORTIFY_SOURCE=2 -fstack-protector-strong")
elseif (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
endif ()

find_package(Backtrace)
include_directories(SYSTEM ${Backtrace_INCLUDE_DIRS})
include_directories(SYSTEM "/usr/lib/gcc/x86_64-linux-gnu/9/include/")

# add conan package manager
include(cmake/conan.cmake)
conan_cmake_run(CONANFILE cmake/conanfile.txt BASIC_SETUP CMAKE_TARGETS BUILD missing)
#==========================

# adding cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

#==========================

# adding Boost to project
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_NO_BOOST_CMAKE TRUE)
find_package(Boost REQUIRED COMPONENTS system exception log log_setup program_options serialization thread unit_test_framework)
include_directories(SYSTEM ${Boost_INCLUDE_DIR})

# Macro required for linking with dl on POSIX and backtrace libraries
add_compile_definitions(BOOST_STACKTRACE_USE_BACKTRACE)

#==========================

# adding OpenSSL to project
find_package(OpenSSL REQUIRED)

#==========================

# adding LevelDB to project
find_package(leveldb CONFIG REQUIRED)

#==========================

# adding Secp256k1 to project
find_package(Secp256k1 REQUIRED)
include_directories(SYSTEM ${SECP256K1_INCLUDE_DIR})

#==========================

# setting up common include directories
include_directories(./src)

#===========================

add_subdirectory(./src)
add_subdirectory(./test/unit_test)

get_cmake_property(_variableNames VARIABLES)
list (SORT _variableNames)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()

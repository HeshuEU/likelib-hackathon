cmake_minimum_required(VERSION 3.15)

project(likelib VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# adding Boost to project
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED COMPONENTS system exception log program_options serialization unit_test_framework)
include_directories(${Boost_INCLUDE_DIR})

# without following definition it will print addresses insted of function names durung stack dump
if("${CMAKE_BUILD_TYPE}" MATCHES "Debug")
    add_compile_definitions(BOOST_STACKTRACE_USE_BACKTRACE)
endif("${CMAKE_BUILD_TYPE}" MATCHES "Debug")

# adding grpc
#list(APPEND CMAKE_PREFIX_PATH "/opt/grpc" "/opt/protobuf")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
find_package(Protobuf REQUIRED)
find_package(GRPC REQUIRED)

#==========================

# setting up common include directories
include_directories(./src)
#===========================

add_subdirectory(./src)
add_subdirectory(./test)

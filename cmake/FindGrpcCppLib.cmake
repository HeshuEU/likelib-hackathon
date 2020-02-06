function(GRPC_GENERATE_CPP SRCS HDRS DEST)
    if(NOT ARGN)
        message(SEND_ERROR "Error: GRPC_GENERATE_CPP() called without any proto files")
        return()
    endif()

    list (LENGTH ARGN __ARGS_COUNT)
    if (${__ARGS_COUNT} GREATER 1)
        message(SEND_ERROR "To much arguments")
        return()
    endif()

    IF (NOT EXISTS ${DEST})
        file(MAKE_DIRECTORY ${DEST})
    ENDIF ()

    set(${SRCS})
    set(${HDRS})
    foreach(FIL ${ARGN})
        get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
        get_filename_component(FIL_WE ${FIL} NAME_WE)
        get_filename_component(FIL_DIR ${FIL} DIRECTORY)

        execute_process(COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} --proto_path=${FIL_DIR} --grpc_out=${DEST} --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN} ${ABS_FIL})
        list(APPEND ${SRCS} "${DEST}/${FIL_WE}.grpc.pb.cc")
        list(APPEND ${HDRS} "${DEST}/${FIL_WE}.grpc.pb.h")

        execute_process(COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} --proto_path=${FIL_DIR} --cpp_out=${DEST} ${ABS_FIL})
        list(APPEND ${SRCS} "${DEST}/${FIL_WE}.pb.cc")
        list(APPEND ${HDRS} "${DEST}/${FIL_WE}.pb.h")
    endforeach()

    set_source_files_properties(${${SRCS}} ${${HDRS}} PROPERTIES GENERATED TRUE)
    set(${SRCS} ${${SRCS}} PARENT_SCOPE)
    set(${HDRS} ${${HDRS}} PARENT_SCOPE)
endfunction()

include(${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake)

# Find the Protobuf include directory
find_path(PROTOBUF_INCLUDE_DIR google/protobuf/service.h)
mark_as_advanced(PROTOBUF_INCLUDE_DIR)


# The Protobuf library
find_library(PROTOBUF_LIBRARY NAMES protobufd)
mark_as_advanced(PROTOBUF_LIBRARY)
add_library(protobuf::libprotobuf UNKNOWN IMPORTED)
set_target_properties(protobuf::libprotobuf PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${PROTOBUF_INCLUDE_DIR}
        INTERFACE_LINK_LIBRARIES pthread
        IMPORTED_LOCATION ${PROTOBUF_LIBRARY}
        )


# Find the protoc Executable
find_program(PROTOBUF_PROTOC_EXECUTABLE NAMES protoc)
mark_as_advanced(PROTOBUF_PROTOC_EXECUTABLE)
add_executable(protobuf::protoc IMPORTED)
set_target_properties(protobuf::protoc PROPERTIES
        IMPORTED_LOCATION ${PROTOBUF_PROTOC_EXECUTABLE}
        )


FIND_PACKAGE_HANDLE_STANDARD_ARGS(Protobuf DEFAULT_MSG
        PROTOBUF_LIBRARY PROTOBUF_INCLUDE_DIR
        PROTOBUF_PROTOC_EXECUTABLE
        )


# Find gRPC include directory
find_path(GRPC_INCLUDE_DIR grpc/grpc.h)
mark_as_advanced(GRPC_INCLUDE_DIR)


# Find c ares library
find_library(GRPC_CARES_LIBRARY NAMES cares)
mark_as_advanced(GRPC_CARES_LIBRARY)
add_library(cares::cares UNKNOWN IMPORTED)
set_target_properties(cares::cares PROPERTIES
        IMPORTED_LOCATION ${GRPC_CARES_LIBRARY}
        )


FIND_PACKAGE_HANDLE_STANDARD_ARGS(cares DEFAULT_MSG
        GRPC_CARES_LIBRARY)


# Find z library
find_library(GRPC_ZLIB_LIBRARY NAMES z)
mark_as_advanced(GRPC_ZLIB_LIBRARY)
add_library(zlib::zlib UNKNOWN IMPORTED)
set_target_properties(zlib::zlib PROPERTIES
        IMPORTED_LOCATION ${GRPC_ZLIB_LIBRARY}
        )


FIND_PACKAGE_HANDLE_STANDARD_ARGS(z DEFAULT_MSG
        GRPC_ZLIB_LIBRARY)


# Find gRPC library
find_library(GRPC_LIBRARY NAMES grpc)
mark_as_advanced(GRPC_LIBRARY)
add_library(gRPC::grpc UNKNOWN IMPORTED)
set_target_properties(gRPC::grpc PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${GRPC_INCLUDE_DIR}
        INTERFACE_LINK_LIBRARIES "-lpthread;-ldl"
        IMPORTED_LOCATION ${GRPC_LIBRARY}
        )


# Find gRPC C++ library
find_library(GRPC_GRPC++_LIBRARY NAMES grpc++)
mark_as_advanced(GRPC_GRPC++_LIBRARY)
add_library(gRPC::grpc++ UNKNOWN IMPORTED)
set_target_properties(gRPC::grpc++ PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${GRPC_INCLUDE_DIR}
        INTERFACE_LINK_LIBRARIES gRPC::grpc
        IMPORTED_LOCATION ${GRPC_GRPC++_LIBRARY}
        )


# Find gRPC C++ reflection library
find_library(GRPC_GRPC++_REFLECTION_LIBRARY NAMES grpc++_reflection)
mark_as_advanced(GRPC_GRPC++_REFLECTION_LIBRARY)
add_library(gRPC::grpc++_reflection UNKNOWN IMPORTED)
set_target_properties(gRPC::grpc++_reflection PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${GRPC_INCLUDE_DIR}
        INTERFACE_LINK_LIBRARIES gRPC::grpc++
        IMPORTED_LOCATION ${GRPC_GRPC++_REFLECTION_LIBRARY}
        )


# Find gpr library
find_library(GRPC_GPR_LIBRARY NAMES gpr)
mark_as_advanced(GRPC_GPR_LIBRARY)
add_library(gRPC::gpr UNKNOWN IMPORTED)
set_target_properties(gRPC::gpr PROPERTIES
        IMPORTED_LOCATION ${GRPC_GPR_LIBRARY}
        )


# Find upb library
find_library(GRPC_UPD_LIBRARY NAMES upb)
mark_as_advanced(GRPC_UPD_LIBRARY)
add_library(gRPC::upb UNKNOWN IMPORTED)
set_target_properties(gRPC::upb PROPERTIES
        IMPORTED_LOCATION ${GRPC_UPD_LIBRARY}
        )


# Find upb library
find_library(GRPC_GRPC_CRONET_LIBRARY NAMES grpc_cronet)
mark_as_advanced(GRPC_GRPC_CRONET_LIBRARY)
add_library(gRPC::grpc_cronet UNKNOWN IMPORTED)
set_target_properties(gRPC::grpc_cronet PROPERTIES
        IMPORTED_LOCATION ${GRPC_GRPC_CRONET_LIBRARY}
        )


# Find address sorting library
find_library(GRPC_ADDRESS_SORTING_LIBRARY NAMES address_sorting)
mark_as_advanced(GRPC_ADDRESS_SORTING_LIBRARY)
add_library(gRPC::address_sorting UNKNOWN IMPORTED)
set_target_properties(gRPC::address_sorting PROPERTIES
        IMPORTED_LOCATION ${GRPC_ADDRESS_SORTING_LIBRARY}
        )


# Find gRpc unsecure library
find_library(GRPC_GRPC_UNSECURE_LIBRARY NAMES grpc_unsecure)
mark_as_advanced(GRPC_GRPC_UNSECURE_LIBRARY)
add_library(gRPC::grpc_unsecure UNKNOWN IMPORTED)
set_target_properties(gRPC::grpc_unsecure PROPERTIES
        IMPORTED_LOCATION ${GRPC_GRPC_UNSECURE_LIBRARY}
        )


# Find gRpc unsecure C++ library
find_library(GRPC_GRPC_CPP_UNSECURE_LIBRARY NAMES grpc++_unsecure)
mark_as_advanced(GRPC_GRPC_CPP_UNSECURE_LIBRARY)
add_library(gRPC::grpc++_unsecure UNKNOWN IMPORTED)
set_target_properties(gRPC::grpc++_unsecure PROPERTIES
        IMPORTED_LOCATION ${GRPC_GRPC_CPP_UNSECURE_LIBRARY}
        )


# Find gRpc C++ channelz library
find_library(GRPC_GRPC_CPP_CHANNELZ_LIBRARY NAMES grpcpp_channelz)
mark_as_advanced(GRPC_GRPC_CPP_CHANNELZ_LIBRARY)
add_library(gRPC::grpcpp_channelz UNKNOWN IMPORTED)
set_target_properties(gRPC::grpcpp_channelz PROPERTIES
        IMPORTED_LOCATION ${GRPC_GRPC_CPP_CHANNELZ_LIBRARY}
        )


# Find gRpc C++ error details library
find_library(GRPC_GRPC_CPP_ERROR_DETAILES_LIBRARY NAMES grpc++_error_details)
mark_as_advanced(GRPC_GRPC_CPP_ERROR_DETAILES_LIBRARY)
add_library(gRPC::grpc++_error_details UNKNOWN IMPORTED)
set_target_properties(gRPC::grpc++_error_details PROPERTIES
        IMPORTED_LOCATION ${GRPC_GRPC_CPP_ERROR_DETAILES_LIBRARY}
        )


# Find gRPC CPP generator
find_program(GRPC_CPP_PLUGIN NAMES grpc_cpp_plugin)
mark_as_advanced(GRPC_CPP_PLUGIN)
add_executable(gRPC::grpc_cpp_plugin IMPORTED)
set_target_properties(gRPC::grpc_cpp_plugin PROPERTIES
        IMPORTED_LOCATION ${GRPC_CPP_PLUGIN}
        )


FIND_PACKAGE_HANDLE_STANDARD_ARGS(gRPC DEFAULT_MSG
        GRPC_LIBRARY GRPC_INCLUDE_DIR GRPC_GRPC++_REFLECTION_LIBRARY GRPC_CPP_PLUGIN)


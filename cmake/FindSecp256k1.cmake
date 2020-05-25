include(${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake)

find_path(SECP256K1_INCLUDE_DIR include/secp256k1_recovery.h)
mark_as_advanced(SECP256K1_INCLUDE_DIR)

find_library(SECP256K1_LIBS NAMES secp256k1)
mark_as_advanced(SECP256K1_LIBS)
add_library(secp256k1_lib UNKNOWN IMPORTED)
set_target_properties(secp256k1_lib PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${SECP256K1_INCLUDE_DIR}
        IMPORTED_LOCATION ${SECP256K1_LIBS}
        )

FIND_PACKAGE_HANDLE_STANDARD_ARGS(secp256k1 DEFAULT_MSG SECP256K1_LIBS)

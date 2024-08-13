# FindONNXRuntime.cmake

# Check if NN_MODULE option is enabled
if(NN_MODULE)
    message(STATUS "NN_MODULE is enabled. Looking for ONNX Runtime.")

    # Define a list of possible root paths where ONNX Runtime might be located
    set(ONNXRUNTIME_POSSIBLE_ROOT_PATHS
        "${CMAKE_SOURCE_DIR}/../onnxruntime-win-x64"
        "C:/Program Files/onnxruntime"
        "/usr/local/onnxruntime"
        "/opt/onnxruntime"
        "$ENV{HOME}/onnxruntime"
    )

    # Find the include directory using find_path with HINTS
    find_path(ONNXRUNTIME_INCLUDE_DIR
        NAMES "onnxruntime_cxx_api.h"
        HINTS ${ONNXRUNTIME_POSSIBLE_ROOT_PATHS}
        PATH_SUFFIXES "include"
    )

    if(NOT ONNXRUNTIME_INCLUDE_DIR)
        message(FATAL_ERROR "ONNX Runtime include directory not found!")
    else()
        message(STATUS "Found ONNX Runtime include directory: ${ONNXRUNTIME_INCLUDE_DIR}")
    endif()

    # Find the library using find_library with HINTS
    find_library(ONNXRUNTIME_LIB
        NAMES "onnxruntime.lib"
        HINTS ${ONNXRUNTIME_POSSIBLE_ROOT_PATHS}
        PATH_SUFFIXES "lib"
    )

    if(NOT ONNXRUNTIME_LIB)
        message(FATAL_ERROR "ONNX Runtime library not found!")
    else()
        message(STATUS "Found ONNX Runtime library: ${ONNXRUNTIME_LIB}")
    endif()

    # Create a linkable target for ONNX Runtime
    add_library(onnxruntime STATIC IMPORTED)
    set_target_properties(onnxruntime PROPERTIES
        IMPORTED_LOCATION "${ONNXRUNTIME_LIB}"
        INTERFACE_INCLUDE_DIRECTORIES "${ONNXRUNTIME_INCLUDE_DIR}"
    )

else()
    message(STATUS "NN_MODULE is disabled. Skipping ONNX Runtime.")
endif()

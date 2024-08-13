cmake_minimum_required(VERSION 3.20)

project(libEngine)

set(CMAKE_CXX_STANDARD 17)

option(NN_MODULE "Build engine component that depends on ONNX runtime library" ON)

add_subdirectory("Deps/glfw")
add_subdirectory("Deps/glad")
add_subdirectory("Deps/glm")
add_subdirectory("Deps/imgui")
add_subdirectory("Deps/stb")

# onnx runtime
include("cmake/FindONNXRuntime.cmake")

# glslang
include("cmake/GLSLangConfig.cmake")
add_subdirectory("Deps/glslang")

# assimp
include("cmake/AssimpConfig.cmake")
add_subdirectory("Deps/assimp")

include_directories(
  "${PROJECT_SOURCE_DIR}/"
  "${PROJECT_SOURCE_DIR}/Deps/headers/")

set(aEngine_DEPS glad glfw stb glm imgui assimp ${GLSLANGLIBS})
set(aEngine_SOURCE_DIRS
  ${PROJECT_SOURCE_DIR}/Base
  ${PROJECT_SOURCE_DIR}/System
  ${PROJECT_SOURCE_DIR}/Utils
  ${PROJECT_SOURCE_DIR}/Component
)

if (${NN_MODULE})
  list(APPEND aEngine_SOURCE_DIRS ${PROJECT_SOURCE_DIR}/NN)
  list(APPEND aEngine_DEPS onnxruntime)
endif()

file(GLOB aEngine_SOURCES *.cpp)
foreach(DIR ${aEngine_SOURCE_DIRS})
  file(GLOB_RECURSE CPP_FILES "${DIR}/*.cpp")
  list(APPEND aEngine_SOURCES ${CPP_FILES})
endforeach()

add_library(${PROJECT_NAME} ${aEngine_SOURCES})
target_link_libraries(${PROJECT_NAME} ${aEngine_DEPS})
target_include_directories(
  ${PROJECT_NAME}
  PUBLIC 
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/Deps/headers)

if (${NN_MODULE})
  # Create a global macro visible to all executables linking the engine
  target_compile_definitions(${PROJECT_NAME} PUBLIC ONNX_RUNTIME_FOUND)
endif()
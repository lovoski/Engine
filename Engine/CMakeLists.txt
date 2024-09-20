cmake_minimum_required(VERSION 3.20)

project(libEngine)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory("Deps/eigen3")
add_subdirectory("Deps/glfw")
add_subdirectory("Deps/glad")
add_subdirectory("Deps/imgui")
add_subdirectory("Deps/stb")
add_subdirectory("Deps/loguru")
add_subdirectory("Deps/filedialog")
add_subdirectory("Deps/cereal")
add_subdirectory("Deps/ufbx")

# config file
set(ASSETS_PATH ${PROJECT_SOURCE_DIR}/Assets)
configure_file(Config.h.in EngineConfig.h)

# boost
find_package(Boost CONFIG REQUIRED
  COMPONENTS asio)

# cgal
find_package(CGAL CONFIG REQUIRED)

include_directories(
  "${PROJECT_SOURCE_DIR}/"
  "${PROJECT_SOURCE_DIR}/Deps/headers/")

set(aEngine_DEPS glad glfw stb imgui loguru filedialog cereal ufbx Eigen3::Eigen Boost::asio CGAL::CGAL)
set(aEngine_SOURCE_DIRS
  ${PROJECT_SOURCE_DIR}/Base
  ${PROJECT_SOURCE_DIR}/System
  ${PROJECT_SOURCE_DIR}/Function
  ${PROJECT_SOURCE_DIR}/Component
  ${PROJECT_SOURCE_DIR}/Scripts
)

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
  ${PROJECT_BINARY_DIR}
  ${PROJECT_SOURCE_DIR}/Deps/headers)

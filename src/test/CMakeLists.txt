cmake_minimum_required(VERSION 3.20)

project(EngineTest)

include_directories(
  ${CMAKE_SOURCE_DIR}/src/resource/geometry)

add_executable(test_mesh
  geometry/test_mesh.cpp)
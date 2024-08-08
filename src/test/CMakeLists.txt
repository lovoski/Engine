cmake_minimum_required(VERSION 3.20)

project(EngineTest)

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/resource/geometry)

add_executable(test_mesh
  geometry/test_mesh.cpp)

add_executable(test_motion
  test_motion.cpp
  ${CMAKE_SOURCE_DIR}/src/resource/animation/Motion.cpp)
target_link_libraries(test_motion PRIVATE glm)

add_executable(test_reflection
  test_reflection.cpp)
set_property(TARGET test_reflection PROPERTY CXX_STANDARD 17)
target_link_libraries(test_reflection PRIVATE ${ENGINE_DEPS})
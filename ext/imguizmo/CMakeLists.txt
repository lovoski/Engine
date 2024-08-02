cmake_minimum_required(VERSION 3.20)

project(ImGuizmo)

add_library(${PROJECT_NAME}
  GraphEditor.cpp
  ImCurveEdit.cpp
  ImGradient.cpp
  ImGuizmo.cpp
  ImSequencer.cpp)
target_include_directories(
  ${PROJECT_NAME} PUBLIC
  "${PROJECT_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}/ext/imgui")
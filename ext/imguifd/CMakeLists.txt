cmake_minimum_required(VERSION 3.20)

project(ImGuiFileDialog)

include_directories(
  "${CMAKE_SOURCE_DIR}/ext/imgui")
add_library(${PROJECT_NAME}
  ImGuiFileDialog.cpp)
target_include_directories(
  ${PROJECT_NAME} 
  PUBLIC 
  ${PROJECT_SOURCE_DIR})
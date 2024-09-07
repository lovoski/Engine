cmake_minimum_required(VERSION 3.20)

project(imgui)

file(GLOB IMGUI_SRC_FILES *.cpp)

list(APPEND IMGUI_SRC_FILES 
  backends/imgui_impl_glfw.cpp 
  backends/imgui_impl_opengl3.cpp)

# use imgui for opengl glfw only
add_library(imgui ${IMGUI_SRC_FILES})

target_include_directories(imgui PUBLIC
  "${PROJECT_SOURCE_DIR}/"
  "${PROJECT_SOURCE_DIR}/backends"
  "${PROJECT_SOURCE_DIR}/../glfw/include"
  "${PROJECT_SOURCE_DIR}/../glad/include")
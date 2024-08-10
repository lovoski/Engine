cmake_minimum_required(VERSION 3.20)

project(imgui)

add_library(imgui
  imgui.cpp
  imgui_demo.cpp
  imgui_draw.cpp
  imgui_tables.cpp
  imgui_widgets.cpp
  backends/imgui_impl_glfw.cpp
  backends/imgui_impl_opengl3.cpp
  GraphEditor.cpp
  ImCurveEdit.cpp
  ImGradient.cpp
  ImGuiFileDialog.cpp
  ImGuizmo.cpp
  ImSequencer.cpp
)
target_include_directories(imgui PUBLIC 
  "${PROJECT_SOURCE_DIR}/"
  "${PROJECT_SOURCE_DIR}/backends"
  "${PROJECT_SOURCE_DIR}/../glfw/include"
  "${PROJECT_SOURCE_DIR}/../glad/include")
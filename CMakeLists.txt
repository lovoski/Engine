cmake_minimum_required(VERSION 3.20)

project(Engine)

set(CMAKE_CXX_STANDARD 17)

option(BUILD_ENGINE_TEST "Build test examples for the engine" ON)

add_subdirectory("ext/glad")
add_subdirectory("ext/glfw")
add_subdirectory("ext/imgui")
add_subdirectory("ext/stb")
add_subdirectory("ext/assimp")
add_subdirectory("ext/glm")
add_subdirectory("ext/imguizmo")
add_subdirectory("ext/imguifd")

set(REPO_SOURCE_DIR ${CMAKE_SOURCE_DIR})
configure_file(config.h.in EngineConfig.h)

set(ENGINE_DEPS glfw glad assimp stb glm imgui ImGuizmo ImGuiFileDialog)

if (BUILD_ENGINE_TEST)
  add_subdirectory("src/test")
endif()

include_directories(
  "${CMAKE_BINARY_DIR}"
  "${CMAKE_SOURCE_DIR}/src")

add_executable(${PROJECT_NAME} 
  src/main.cpp
  src/engine/Engine.cpp
  src/engine/Timer.cpp
  src/engine/Events.cpp
  src/engine/Console.cpp
  src/ecs/systems/gui/GuiSystem.cpp
  src/ecs/systems/gui/window/AssetsWindow.cpp
  src/ecs/systems/gui/window/EntitiesWindow.cpp
  src/ecs/systems/gui/window/InspectorWindow.cpp
  src/utils/Math.cpp
  src/resource/ResourceManager.cpp
  src/resource/MaterialData.cpp
  src/resource/animation/Motion.cpp
  src/ecs/base/Library.cpp
  src/ecs/systems/render/FrameBuffer.cpp)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
target_link_libraries(${PROJECT_NAME} PRIVATE ${ENGINE_DEPS})

# copy resources to relative path of binaries
file(GLOB_RECURSE RESOURCE_FILES ${CMAKE_SOURCE_DIR}/src/default/**)
foreach(RESOURCE_FILE ${RESOURCE_FILES})
  file(RELATIVE_PATH RESOURCE_REL_PATH ${CMAKE_SOURCE_DIR}/src ${RESOURCE_FILE})
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND
  ${CMAKE_COMMAND} -E copy_if_different ${RESOURCE_FILE}
  $<TARGET_FILE_DIR:${PROJECT_NAME}>/${RESOURCE_REL_PATH})
endforeach()
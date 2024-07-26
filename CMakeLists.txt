cmake_minimum_required(VERSION 3.20)

project(Engine)

option(BUILD_ENGINE_TEST "Build test examples for the engine" ON)

add_subdirectory("ext/glad")
add_subdirectory("ext/glfw")
add_subdirectory("ext/imgui")
add_subdirectory("ext/json11")
add_subdirectory("ext/stb")
add_subdirectory("ext/assimp")
add_subdirectory("ext/glm")
add_subdirectory("ext/rttr")

if (BUILD_ENGINE_TEST)
  add_subdirectory("src/test")
endif()

set(REPO_SOURCE_DIR "${CMAKE_SOURCE_DIR}")
configure_file(config.h.in EngineConfig.h)

set(ENGINE_DEPS glfw glad assimp stb json11 glm imgui rttr_core)

include_directories(
  "${CMAKE_BINARY_DIR}"
  "${CMAKE_SOURCE_DIR}/src")

add_executable(${PROJECT_NAME} 
  src/main.cpp
  src/engine/Engine.cpp
  src/engine/Timer.cpp
  src/engine/Events.cpp
  src/engine/Console.cpp
  src/utils/Math.cpp
  src/resource/ResourceManager.cpp
  src/ecs/base/Types.cpp
  src/ecs/components/Transform.cpp
  src/ecs/systems/render/FrameBuffer.cpp
  src/ecs/systems/render/EditorWindows.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE ${ENGINE_DEPS})

cmake_minimum_required(VERSION 3.20)

project(aEngineEditor)

set(CMAKE_CXX_STANDARD 17)

set(editor_SOURCE_DIRS
  ${CMAKE_SOURCE_DIR}/Editor
)
set(editor_SOURCES)
foreach(DIR ${editor_SOURCE_DIRS})
  file(GLOB_RECURSE CPP_FILES "${DIR}/*.cpp")
  list(APPEND editor_SOURCES ${CPP_FILES})
endforeach()

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

add_subdirectory(Engine)

add_executable(${PROJECT_NAME} ${editor_SOURCES})
target_link_libraries(${PROJECT_NAME} libEngine)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

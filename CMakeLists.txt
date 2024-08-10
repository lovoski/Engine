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

add_subdirectory(Engine)

add_executable(${PROJECT_NAME} ${editor_SOURCES})
target_link_libraries(${PROJECT_NAME} libEngine)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

# copy default assets file to relative path of editor
file(GLOB_RECURSE RESOURCE_FILES ${CMAKE_SOURCE_DIR}/Engine/Assets/**)
foreach(RESOURCE_FILE ${RESOURCE_FILES})
  file(RELATIVE_PATH RESOURCE_REL_PATH ${CMAKE_SOURCE_DIR}/Engine ${RESOURCE_FILE})
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND
  ${CMAKE_COMMAND} -E copy_if_different ${RESOURCE_FILE}
  $<TARGET_FILE_DIR:${PROJECT_NAME}>/${RESOURCE_REL_PATH})
endforeach()
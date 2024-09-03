cmake_minimum_required(VERSION 3.20)

project(filedialog)

add_library(${PROJECT_NAME} tinyfiledialogs.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

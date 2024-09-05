cmake_minimum_required(VERSION 3.20)

project(ufbx)

add_library(${PROJECT_NAME} ufbx.c)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR})

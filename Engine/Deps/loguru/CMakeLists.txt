cmake_minimum_required(VERSION 3.20)

project(loguru)

add_library(loguru loguru.cpp)
target_include_directories(loguru PUBLIC ${PROJECT_SOURCE_DIR})
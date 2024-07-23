cmake_minimum_required(VERSION 3.20)
cmake_policy(SET CMP0048 NEW)
project(json11 VERSION 1.0.0 LANGUAGES CXX)

option(JSON11_ENABLE_DR1467_CANARY "Enable canary test for DR 1467" OFF)

if(CMAKE_VERSION VERSION_LESS "3")
  add_definitions(-std=c++11)
else()
  set(CMAKE_CXX_STANDARD 11)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX /usr)
endif()

add_library(json11 json11.cpp)
target_include_directories(json11 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
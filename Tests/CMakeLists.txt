cmake_minimum_required(VERSION 3.20)

project(EngineTests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_executable(test_kdtree Datastructure/kdtree.cpp)
target_link_libraries(test_kdtree PUBLIC libEngine)

add_executable(test_lafan_formalize Processing/formalize_lafan.cpp)
target_link_libraries(test_lafan_formalize PUBLIC libEngine)
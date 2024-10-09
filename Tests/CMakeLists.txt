cmake_minimum_required(VERSION 3.20)

project(EngineTests)

add_executable(test_kdtree Datastructure/kdtree.cpp)
target_link_libraries(test_kdtree PUBLIC libEngine)
set_property(TARGET test_kdtree PROPERTY CXX_STANDARD 17)

cmake_minimum_required(VERSION 2.8)
project(examples)

add_executable(armor_extractor ./armor_extractor.cpp)

target_link_libraries(armor_extractor perception)

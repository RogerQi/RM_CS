cmake_minimum_required(VERSION 2.8)
project(perception)

set(SOURCES camera.cpp rune_bot.cpp)

add_library(perception ${SOURCES})

target_link_libraries(perception ${OpenCV_LIBS} glog boost_system pthread jetson-inference)

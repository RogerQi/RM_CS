cmake_minimum_required(VERSION 2.8)
project(cvtoolbox)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

find_package(OpenCV REQUIRED)

add_compile_options(-std=c++11)

include_directories(${OpenCV_INLUCDE_DIRS})
include_directories(.)

set(SOURCES camera.cpp rune.cpp)
add_executable(single_shot ${SOURCES} test/single_shot.cpp)
add_executable(digit_recog ${SOURCES} test/digit_recog.cpp)

target_link_libraries(single_shot ${OpenCV_LIBS})
target_link_libraries(digit_recog ${OpenCV_LIBS})

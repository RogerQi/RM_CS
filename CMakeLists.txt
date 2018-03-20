cmake_minimum_required(VERSION 2.8)
project(IRM)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

option(USE_GPU "set to ON to use GPU" OFF)
option(DEBUG "set to ON to show debug information and image plot" OFF)

add_compile_options(-std=c++11)

find_package(OpenCV REQUIRED)
find_package(Caffe REQUIRED)

include_directories(${OpenCV_INLUCDE_DIRS})
include_directories(${Caffe_INCLUDE_DIRS})
include_directories(/usr/local/opt/openblas/include)
include_directories(${CMAKE_SOURCE_DIR}/perception)
include_directories(${CMAKE_SOURCE_DIR}/communication)

if (USE_GPU)
    message("Caffe running in GPU Mode")
else()
    add_definitions(-DCPU_ONLY=1)
    message("Caffe running in CPU Mode")
endif()

if (DEBUG)
    add_definitions(-DDEBUG)
endif()

add_subdirectory(perception)

add_executable(single_shot ./example/single_shot.cpp)
add_executable(digit_recog ./example/digit_recog.cpp)
add_executable(armor_detect ./example/video_armor_detector.cpp)

target_link_libraries(single_shot perception)
target_link_libraries(digit_recog perception)
target_link_libraries(armor_detect perception)

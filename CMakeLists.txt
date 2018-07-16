cmake_minimum_required(VERSION 2.8)
project(IRM)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

option(USE_GPU "set to ON to use GPU" OFF)
option(DEBUG "set to ON to show debug information and image plot" OFF)

add_compile_options(-std=c++11)
add_compile_options(-g)

find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)

include_directories(${OpenCV_INLUCDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/perception)
include_directories(${CMAKE_SOURCE_DIR}/communication)
include_directories(${CMAKE_SOURCE_DIR}/tensorrt)

find_package(CUDA REQUIRED)
include_directories(${CUDA_INCLUDE_DIRS})

if (USE_GPU)
    message("Running in GPU Mode")
else()
    add_definitions(-DCPU_ONLY=1)
    message("Running in CPU Mode")
endif()

if (DEBUG)
    add_definitions(-DDEBUG)
endif()

add_subdirectory(perception)
add_subdirectory(communication)
add_subdirectory(examples)
add_subdirectory(tools)
add_subdirectory(tensorrt)

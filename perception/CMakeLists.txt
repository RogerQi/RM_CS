cmake_minimum_required(VERSION 2.8)
project(cvtoolbox)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(GENERIC_LIBS glog boost_system m)

option(USE_GPU "set to ON to use GPU" OFF)
option(DEBUG "set to ON to show debug information and image plot" OFF)

find_package(OpenCV REQUIRED)
find_package(Caffe REQUIRED)

add_compile_options(-std=c++11)

include_directories(${OpenCV_INLUCDE_DIRS})
include_directories(${Caffe_INCLUDE_DIRS})
include_directories(/usr/local/opt/openblas/include)
include_directories(.)

set(SOURCES camera.cpp rune.cpp)
add_executable(single_shot ${SOURCES} example/single_shot.cpp)
add_executable(digit_recog ${SOURCES} example/digit_recog.cpp)

set(DETECT_EXAMPLE_SOURCES camera.cpp aimbot.cpp)
add_executable(armor_detect ${DETECT_EXAMPLE_SOURCES} example/video_armor_detector.cpp)

if (USE_GPU)
    message("Caffe running in GPU Mode")
else()
    add_definitions(-DCPU_ONLY=1)
    message("Caffe running in CPU Mode")
endif()

if (DEBUG)
    add_definitions(-DDEBUG)
endif()

target_link_libraries(single_shot ${OpenCV_LIBS} ${Caffe_LIBRARIES} ${GENERIC_LIBS})
target_link_libraries(digit_recog ${OpenCV_LIBS} ${Caffe_LIBRARIES} ${GENERIC_LIBS})

target_link_libraries(armor_detect ${OpenCV_LIBS} ${GENERIC_LIBS})

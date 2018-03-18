cmake_minimum_required(VERSION 2.8)
project(cvtoolbox)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

find_package(OpenCV REQUIRED)

add_compile_options(-std=c++11)

include_directories(${OpenCV_INLUCDE_DIRS})

set(SOURCES camera.cpp)
add_executable(cam_test ${SOURCES} cam_test.cpp)

target_link_libraries(cam_test ${OpenCV_LIBS})

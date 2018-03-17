cmake_minimum_required(VERSION 2.8)
project(camera)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

find_package(OpenCV REQUIRED)

add_compile_options(-std=c++11)

include_directories(${OpenCV_INLUCDE_DIRS})

set(SOURCES main.cpp camera.cpp)
add_executable(camera ${SOURCES})

target_link_libraries(camera ${OpenCV_LIBS})

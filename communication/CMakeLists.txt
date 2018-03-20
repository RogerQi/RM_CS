cmake_minimum_required(VERSION 2.8)
project(communication)

set(SOURCES serial.cpp protocol.cpp)

add_library(communication ${SOURCES})

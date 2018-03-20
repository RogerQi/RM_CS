cmake_minimum_required(VERSION 2.8)
project(examples)

add_executable(single_shot ./single_shot.cpp)
add_executable(digit_recog ./digit_recog.cpp)
add_executable(armor_detect ./video_armor_detector.cpp)
add_executable(serial_test ./serial_test.cpp)

target_link_libraries(single_shot perception)
target_link_libraries(digit_recog perception)
target_link_libraries(armor_detect perception)
target_link_libraries(serial_test communication)

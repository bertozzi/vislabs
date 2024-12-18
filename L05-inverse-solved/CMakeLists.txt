cmake_minimum_required(VERSION 2.8)
project(lab5_2)

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

add_definitions(-std=gnu++0x)

add_executable(lab5_2 main.cpp)

target_link_libraries( lab5_2 ${OpenCV_LIBS} )
install(TARGETS lab5_2 RUNTIME DESTINATION bin)

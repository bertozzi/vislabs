cmake_minimum_required(VERSION 2.8)
project(lab4)

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

add_executable(pinhole main.cpp)

target_link_libraries(pinhole ${OpenCV_LIBS} )
install(TARGETS pinhole RUNTIME DESTINATION bin)

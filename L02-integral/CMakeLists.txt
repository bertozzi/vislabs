cmake_minimum_required(VERSION 2.8)
project(smooth)

find_package(OpenCV REQUIRED)

add_definitions("-std=c++11 -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g")

add_executable(intgr intgr.cpp)
target_link_libraries(intgr ${OpenCV_LIBS})
install(TARGETS intgr RUNTIME DESTINATION bin)



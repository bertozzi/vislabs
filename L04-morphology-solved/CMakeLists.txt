cmake_minimum_required(VERSION 2.8)
project(simple)

find_package(OpenCV REQUIRED)

add_definitions("-std=c++11 -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g")

add_executable(exprunning exprunning.cpp)
target_link_libraries(exprunning ${OpenCV_LIBS})
install(TARGETS exprunning RUNTIME DESTINATION bin)



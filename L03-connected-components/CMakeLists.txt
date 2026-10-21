cmake_minimum_required(VERSION 2.8)
project(cc)

find_package(OpenCV REQUIRED)

add_definitions("-std=c++11 -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g")

add_executable(cc cc.cpp)
target_link_libraries(cc ${OpenCV_LIBS})
install(TARGETS cc RUNTIME DESTINATION bin)

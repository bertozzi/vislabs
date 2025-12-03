cmake_minimum_required(VERSION 2.8)
project(simple)

find_package(OpenCV REQUIRED)

add_definitions("-std=c++11 -g")
#add_definitions("-DNATIVE_OPTS")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g")

add_executable(ransac skel.cpp)
target_link_libraries(ransac ${OpenCV_LIBS})
install(TARGETS ransac RUNTIME DESTINATION bin)


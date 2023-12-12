cmake_minimum_required(VERSION 2.8)
project(kmeans)

find_package(OpenCV REQUIRED)

add_definitions("-std=c++11 -g")
#add_definitions("-DNATIVE_OPTS")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g")

add_executable(kmeans kmeans.cpp)
target_link_libraries(kmeans ${OpenCV_LIBS})
install(TARGETS kmeans RUNTIME DESTINATION bin)


#add_executable(test test.cpp)
#target_link_libraries(test ${OpenCV_LIBS})
#install(TARGETS test RUNTIME DESTINATION bin)

cmake_minimum_required(VERSION 2.8)
project(simple)


find_package(OpenCV 3.4.3 REQUIRED)

add_definitions("-std=c++11")

add_compile_options(
  "-Wall" "-Wpedantic" "-Wextra" "-fexceptions"
  "$<$<CONFIG:DEBUG>:-O0;-g3;-ggdb>"
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

add_executable(simple main.cpp Functions.cpp)
target_link_libraries(simple ${OpenCV_LIBS})
install(TARGETS simple RUNTIME DESTINATION bin)


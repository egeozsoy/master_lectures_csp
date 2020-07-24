cmake_minimum_required(VERSION 3.16)
project(master_csp_cpp)

set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3")
include_directories(/usr/local/include)
find_library(XLNT xlnt)

add_executable(master_csp_cpp main.cpp helpers/lecture.hpp helpers/constraint.hpp)
target_link_libraries(master_csp_cpp PRIVATE "${XLNT}")
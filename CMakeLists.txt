cmake_minimum_required(VERSION 3.16)
project(master_csp_cpp)

set(CMAKE_CXX_STANDARD 14)
include_directories(~/cpp_libraries)
find_library(XLNT xlnt)

add_executable(master_csp_cpp main.cpp helpers/lecture.hpp)
target_link_libraries(master_csp_cpp PRIVATE "${XLNT}")
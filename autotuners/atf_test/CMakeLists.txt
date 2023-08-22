cmake_minimum_required(VERSION 3.10)

# set the project name
project(NoarrATFTest VERSION 0.20.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(../../include)
include_directories(${ATF_HOME})

add_executable(atf_test_direct test_direct.cpp)
add_executable(atf_test_cmake test_cmake.cpp)
add_executable(atf_test_cmake_kernel test_cmake.cpp)

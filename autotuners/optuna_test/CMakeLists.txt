cmake_minimum_required(VERSION 3.10)
Include(FetchContent)

# set the project name
project(NoarrOptunaTest VERSION 0.20.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(optuna_test_direct test_direct.cpp)
target_include_directories(optuna_test_direct PUBLIC ../../include)

add_executable(optuna_test_cmake test_cmake.cpp)
target_include_directories(optuna_test_cmake PUBLIC ../../include)

add_executable(optuna_test_cmake_kernel test_cmake.cpp)
target_include_directories(optuna_test_cmake_kernel PUBLIC ../../include)

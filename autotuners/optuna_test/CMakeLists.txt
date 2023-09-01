cmake_minimum_required(VERSION 3.10)

# set the project name
project(NoarrOptunaTest VERSION 0.20.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(optuna_test_direct test_direct.cpp)
target_include_directories(optuna_test_direct PUBLIC ../../include)
target_compile_definitions(optuna_test_direct PUBLIC NOARR_TUNE)

add_executable(optuna_test_cmake test_cmake.cpp)
target_include_directories(optuna_test_cmake PUBLIC ../../include)
target_compile_definitions(optuna_test_cmake PUBLIC NOARR_TUNE)

add_executable(optuna_test_cmake_kernel test_cmake.cpp)
target_include_directories(optuna_test_cmake_kernel PUBLIC ../../include)

cmake_minimum_required(VERSION 3.10)

include(FetchContent)

# set the project name
project(NoarrTuningDummyTest VERSION 0.20.0)

# fetch Noarr Structures
FetchContent_Declare(
    Noarr
    GIT_REPOSITORY https://github.com/jiriklepl/noarr-structures.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(Noarr)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories("${Noarr_SOURCE_DIR}/include")

add_executable(dummy_test test.cpp)
target_include_directories(dummy_test PUBLIC ../../include)
target_compile_definitions(dummy_test PUBLIC NOARR_TUNE)

add_executable(dummy_test_kernel test.cpp)
target_include_directories(dummy_test_kernel PUBLIC ../../include)

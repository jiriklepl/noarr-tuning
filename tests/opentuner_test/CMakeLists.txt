cmake_minimum_required(VERSION 3.10)

include(FetchContent)

# set the project name
project(NoarrTuningOpenTunerTest VERSION 0.20.0 LANGUAGES CXX)

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

add_executable(opentuner_test_direct test_direct.cpp)
target_include_directories(opentuner_test_direct PUBLIC ../../include)
target_compile_definitions(opentuner_test_direct PUBLIC NOARR_TUNE)

add_executable(opentuner_test_cmake test_cmake.cpp)
target_include_directories(opentuner_test_cmake PUBLIC ../../include)
target_compile_definitions(opentuner_test_cmake PUBLIC NOARR_TUNE)

add_executable(opentuner_test_cmake_kernel test_cmake.cpp)
target_include_directories(opentuner_test_cmake_kernel PUBLIC ../../include)

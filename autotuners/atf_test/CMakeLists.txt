cmake_minimum_required(VERSION 3.10)

include(FetchContent)

# set the project name
project(NoarrATFTest VERSION 0.20.0)

# fetch atf from gitlab
FetchContent_Declare(
    atf
    GIT_REPOSITORY https://gitlab.com/mdh-project/atf.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(atf)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(../../include)
include_directories("${atf_SOURCE_DIR}")

add_executable(atf_test_direct test_direct.cpp)
target_compile_definitions(atf_test_direct PRIVATE NOARR_TUNE)

add_executable(atf_test_cmake test_cmake.cpp)
target_compile_definitions(atf_test_cmake PRIVATE NOARR_TUNE)

add_executable(atf_test_cmake_kernel test_cmake.cpp)

cmake_minimum_required(VERSION 3.10)

project(NoarrTuning
    VERSION 0.20.0
    LANGUAGES CXX
)

add_library(NoarrTuning INTERFACE)

set(NOARR_TUNING_DIR ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(NoarrTuning INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

install(DIRECTORY include/ DESTINATION include)

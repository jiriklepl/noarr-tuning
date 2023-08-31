#!/bin/bash -ex

mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . -j"$(nproc)"

./atf_test_cmake

#!/bin/bash -ex

source ./prepare_env.sh

mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . -j"$(nproc)" --target atf_test_cmake

./atf_test_cmake

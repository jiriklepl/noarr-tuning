#!/bin/bash -ex

if [ ! -d "$ATF_HOME" ]; then
    echo "ATF_HOME is not set"
    exit 1
fi

mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug .. -DATF_HOME=$ATF_HOME
cmake --build . -j"$(nproc)"

./atf_test_cmake

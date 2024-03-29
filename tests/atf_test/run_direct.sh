#!/bin/bash -ex

source ./prepare_env.sh

if [ ! -d "$ATF_HOME" ]; then
    ATF_HOME="$PWD/build/_deps/atf-src"
fi

mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug .. -DCMAKE_CXX_FLAGS="-DATF_HOME=\"$ATF_HOME\" $CMAKE_CXX_FLAGS"
cmake --build . -j"$(nproc)"

./atf_test_direct

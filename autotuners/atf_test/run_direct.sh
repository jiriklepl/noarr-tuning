#!/bin/bash -ex

if [ ! -d env ]; then
    python3 -m venv env
fi

source env/bin/activate
pip install -r requirements.txt

if [ ! -f matrices ]; then
    ./gen-matrices.py matrices 1024
fi

if [ ! -d "$ATF_HOME" ]; then
    ATF_HOME="$PWD/build/_deps/atf-src"
fi

mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug .. -DCMAKE_CXX_FLAGS="-DATF_HOME=\"$ATF_HOME\" $CMAKE_CXX_FLAGS"
cmake --build . -j"$(nproc)"

./atf_test_direct

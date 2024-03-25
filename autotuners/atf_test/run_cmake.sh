#!/bin/bash -ex

if [ ! -d env ]; then
    python3 -m venv env
fi

source env/bin/activate
pip install -r requirements.txt

if [ ! -f matrices ]; then
    ./gen-matrices.py matrices 1024
fi

mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . -j"$(nproc)"

./atf_test_cmake

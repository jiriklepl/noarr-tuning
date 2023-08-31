#!/bin/bash -ex

mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j"$(nproc)"
python <(./opentuner_test_cmake) --test-limit=20 --no-dups

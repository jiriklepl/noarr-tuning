#!/bin/bash -ex

source ./prepare_env.sh

mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j"$(nproc)" --target opentuner_test_cmake
python <(./opentuner_test_cmake) --test-limit=20 --no-dups

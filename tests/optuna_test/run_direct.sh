#!/bin/bash -ex

source ./prepare_env.sh

mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j"$(nproc)" --target optuna_test_direct
python <(./optuna_test_direct) --n-trials=20

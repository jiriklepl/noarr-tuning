#!/bin/bash -ex

source ./prepare_env.sh

mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j"$(nproc)" --target opentuner_test_direct
python <(./opentuner_test_direct) --test-limit=20 --no-dups

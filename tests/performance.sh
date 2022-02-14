#!/bin/env sh
mkdir -p build/
cd build
cmake ..
cmake --build . --target fastfetch-test-performance -j$(nproc)
./fastfetch-test-performance "$@"

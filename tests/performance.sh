#!/bin/env sh
rm -rf tests/build/
mkdir -p tests/build/
cd tests/build/
cmake ../..
cmake --build . --target fastfetch-test-performance -j$(nproc)
./fastfetch-test-performance "$@"

#!/bin/env sh
mkdir -p build/
cd build/
cmake ..
cmake --build . -j$(nproc)
./fastfetch "$@"

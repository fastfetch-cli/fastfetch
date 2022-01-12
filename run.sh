#!/bin/env sh
mkdir -p build/
cd build/
cmake ..
cmake --build . --target fastfetch -j$(nproc)
./fastfetch "$@"

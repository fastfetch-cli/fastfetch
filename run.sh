#!/bin/env sh
mkdir -p build/
cd build/
cmake ..
make -j$(nproc)
./fastfetch "$@"

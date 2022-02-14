#!/bin/env sh
mkdir -p build/
cd build
cmake ..
cmake --build . --target fastfetch-test-strbuf -j$(nproc)
./fastfetch-test-strbuf "$@"

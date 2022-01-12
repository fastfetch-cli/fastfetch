#!/bin/env sh
rm -rf tests/build/
mkdir -p tests/build/
cd tests/build/
cmake ../..
cmake --build . --target fastfetch-test-strbuf -j$(nproc)
./fastfetch-test-strbuf "$@"

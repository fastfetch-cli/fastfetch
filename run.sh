#!/bin/env sh

# make the script exit if any of the commands fail,
# making fastfetch not run if the build failed.
set -e

mkdir -p build/
cd build/
cmake ..
cmake --build . --target fastfetch -j$(nproc)
./fastfetch "$@"

#!/usr/bin/env sh

# make the script exit if any of the commands fail,
# making fastfetch not run if the build failed.
set -e

mkdir -p build/
cd build/
cmake ..

if [ "$OSTYPE" == "linux-gnu" ]; then
    cmake_build_extra_args="-j$(nproc)"
else
    cmake_build_extra_args=""
fi
cmake --build . --target fastfetch ${cmake_build_extra_args}

./fastfetch "$@"

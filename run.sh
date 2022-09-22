#!/usr/bin/env sh

set -e

mkdir -p build/
cd build/

if [ -z "${CMAKE_BUILD_TYPE}" ]; then
    CMAKE_BUILD_TYPE=Release
fi
cmake "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}" ..

if [ -z "$OSTYPE" ] || [ "$OSTYPE" = "linux-gnu" ]; then
    cmake_build_args="-j$(nproc)"
else
    cmake_build_args=""
fi
cmake --build . --target fastfetch ${cmake_build_args}

export LSAN_OPTIONS=suppressions=../tests/lsan.supp
./fastfetch "$@"

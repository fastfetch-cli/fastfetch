#!/usr/bin/env sh

set -e

mkdir -p build/
cd build/

if [ -z "${CMAKE_BUILD_TYPE}" ]; then
    CMAKE_BUILD_TYPE=Release
fi

cmake "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}" ..

kernel_name="$(uname -s)"

case "${kernel_name}" in
    "Linux")
        cmake_build_args="-j$(nproc)"
        ;;
    "Darwin" | *"BSD" | "DragonFly")
        cmake_build_args="-j$(sysctl -n hw.ncpu)"
        ;;
    *)
        cmake_build_args=""
        ;;
esac

cmake --build . --target fastfetch "${cmake_build_args}"

./fastfetch "$@"

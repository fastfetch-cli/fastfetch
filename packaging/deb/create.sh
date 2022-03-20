#!/bin/sh

# This script is intended to be run from the root of the repository.
# If you don't want to use build/ as build directoy, or already build the elsewere, you can give the build directoy as argument.

BUILD_DIR="${1:-build}"
ROOT_DIR="${2:-.}"
PACKAGE_DIR="${BUILD_DIR}/packaging/deb/fastfetch"

# Create missing files
mkdir -p "${BUILD_DIR}" || exit 1

if [ ! -f "${BUILD_DIR}/cmake_install.cmake" ]; then
    cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" || exit 2
    rm -f "${BUILD_DIR}/fastfetch" || exit 3 # Always rebuild the project if we reconfigured it
fi

if [ ! -f "${BUILD_DIR}/fastfetch" ]; then
    cmake --build "${BUILD_DIR}" || exit 4
fi

# Populate the files
cmake --install "${BUILD_DIR}" --prefix "${PACKAGE_DIR}/usr" || exit 5
mkdir -p "${PACKAGE_DIR}/DEBIAN" || exit 6
sed "s/<VERSION>/$("${BUILD_DIR}/fastfetch" --version-raw)/g" "${ROOT_DIR}/packaging/deb/control-template" > "${PACKAGE_DIR}/DEBIAN/control" || exit 7

# Create the package
dpkg-deb --build "${PACKAGE_DIR}" || exit 8

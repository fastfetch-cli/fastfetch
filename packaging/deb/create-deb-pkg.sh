#!/bin/sh

cd "$(dirname "$0")" || exit
echo "cleanup"
rm -rf ./build ./fastfetch fastfetch.deb

echo "building fastfetch"
mkdir build
cd build || exit 1
cmake ../../.. || exit 1
cmake --build . --target fastfetch "-j$(nproc)" || exit 1

echo "generating package contents"
# deb versions must start with a number, hence the 1 added before the rXXX
version="0:1$(./fastfetch --version | cut -d ' ' -f2)-0"
echo "Using package version: $version"
cd ..

cmake --install build --prefix fastfetch/usr || exit 1
mkdir -p fastfetch/DEBIAN
sed "s/<VERSION>/$version/g" control-template > fastfetch/DEBIAN/control

echo "building package"
dpkg-deb --build fastfetch || exit 1
echo "package built."
echo "cleaning up"
rm -rf build fastfetch

mkdir -p build/
cd build/
cmake ..
cmake --build . "-j$(($(nproc)+1))"
./fastfetch "$@"

mkdir -p build/
cd build/
cmake ..
cmake --build .
./fastfetch "$@"

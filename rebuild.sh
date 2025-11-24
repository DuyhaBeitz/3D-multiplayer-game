cmake -S . -B build
cmake --build build -j$(nproc)

ln -sf "$(pwd)/assets" build/

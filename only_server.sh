cmake -S . -B build
cmake --build build -j$(nproc) --target server

ln -sf "$(pwd)/assets" build/

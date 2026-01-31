cmake -S . -B build -DHEADLESS_BUILD=0
cmake --build build -j$(nproc)

ln -sf "$(pwd)/assets" build/

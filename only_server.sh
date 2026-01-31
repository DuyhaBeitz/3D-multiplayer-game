cmake -S . -B build -DHEADLESS_BUILD=1
cmake --build build -j$(nproc) --target server

ln -sf "$(pwd)/assets" build/

cmake -S . -B build -DHEADLESS_BUILD=0 -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

ln -sf "$(pwd)/assets" build/
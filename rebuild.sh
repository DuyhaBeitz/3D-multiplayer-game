cmake -S . -B build
cmake --build build

ln -sf "$(pwd)/assets" build/assets

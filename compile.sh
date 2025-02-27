cmake -S . -B build --toolchain ~/opt/x-tools/arm-pico-eabi/arm-pico-eabi.toolchain.cmake
cmake --build build -j$(nproc)

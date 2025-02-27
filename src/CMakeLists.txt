add_library(liquid liquid.cpp)

add_library(cell cell.cpp)
target_link_libraries(cell liquid)

add_library(pillar pillar.cpp)

add_library(grid grid.cpp)
target_link_libraries(grid cell pillar)

add_library(state state.cpp)

add_executable(main "main.cpp")
target_compile_features(main PRIVATE cxx_std_23)
target_link_libraries(main PRIVATE warnings pico_stdlib hardware_adc grid state)
pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)
pico_add_extra_outputs(main)
pico_set_binary_type(main copy_to_ram)

find_library(../freertos freertos)

add_library(liquid liquid.cpp)

add_library(cell cell.cpp)
target_link_libraries(cell liquid)

add_library(pillar pillar.cpp)

add_library(grid grid.cpp)
target_link_libraries(grid cell pillar)

add_library(input_state input_state.cpp)

add_library(input_interface input_interface.cpp)
target_link_libraries(input_interface grid input_state pico_stdlib)

add_subdirectory(Motor)

add_executable(main "main.cpp")
target_include_directories(main PRIVATE 
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/Motor)

target_compile_features(main PRIVATE cxx_std_23)
target_link_libraries(main PRIVATE 
            warnings 
            pico_stdlib 
            hardware_gpio 
            hardware_irq
            hardware_pwm
            FreeRTOS
            stepper
)

pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)
pico_add_extra_outputs(main)

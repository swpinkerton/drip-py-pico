find_library(../freertos freertos)

add_subdirectory(Motor)
add_subdirectory(DebugTools)

add_executable(main "main.cpp")

target_include_directories(main PRIVATE 
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/Motor
            ${CMAKE_CURRENT_SOURCE_DIR}/DebugTools)

target_compile_features(main PRIVATE cxx_std_23)
target_link_libraries(main PRIVATE 
            warnings 
            pico_stdlib 
            pico_multicore
            hardware_gpio 
            FreeRTOS
            stepper
            debug_tools
)

add_executable(motor_basic main_basic.cpp)
target_include_directories(motor_basic PRIVATE 
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/DebugTools
            ${CMAKE_CURRENT_SOURCE_DIR}/Motor)

target_compile_features(motor_basic PRIVATE cxx_std_23)
target_link_libraries(motor_basic PRIVATE 
            warnings 
            pico_stdlib 
            pico_multicore
            hardware_gpio 
            FreeRTOS
            stepper
            debug_tools
)

add_executable(irq endstop_irq_main.cpp)
target_link_libraries(irq PRIVATE
    pico_stdlib
    hardware_gpio
)


pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)
pico_add_extra_outputs(main)

pico_enable_stdio_usb(motor_basic 1)
pico_enable_stdio_uart(motor_basic 0)
pico_add_extra_outputs(motor_basic)

pico_enable_stdio_usb(irq 1)
pico_enable_stdio_uart(irq 0)
pico_add_extra_outputs(irq)
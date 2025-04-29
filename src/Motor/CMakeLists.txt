add_library(stepper stepper.cpp)

target_include_directories(stepper PRIVATE ../DebugTools)

target_link_libraries(stepper
    pico_stdlib
    pico_multicore
    hardware_pwm
    hardware_irq
    FreeRTOS
    pico_time
    debug_tools
)
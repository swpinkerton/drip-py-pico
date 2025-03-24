add_library(stepper stepper.cpp)
target_link_libraries(stepper
    pico_stdlib
    hardware_pwm
    hardware_irq
    FreeRTOS
    pico_time
)
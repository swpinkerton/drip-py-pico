# Collect all source files in the Motor directory
file(GLOB MOTOR_SOURCES "*.cpp")

# Create a single library for Motor
add_library(stepper STATIC ${MOTOR_SOURCES})

target_include_directories(stepper PUBLIC 
    .
    ../DebugTools
)

target_link_libraries(stepper PRIVATE
    debug_tools
    pico_stdlib
    pico_multicore
    hardware_gpio
    # hardware_irq
)
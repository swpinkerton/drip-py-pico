find_library(../freertos freertos)

add_library(liquid liquid.cpp)

add_library(commands commands.cpp)

add_library(led led.cpp)
target_link_libraries(led hardware_gpio FreeRTOS)


add_library(command_handler command_handler.cpp)
target_link_libraries(command_handler led commands FreeRTOS)

add_library(input_interface input_interface.cpp)
target_link_libraries(input_interface commands pico_stdlib hardware_gpio FreeRTOS)

add_executable(main "main.cpp")
target_compile_features(main PRIVATE cxx_std_23)
target_link_libraries(main PRIVATE warnings pico_stdlib hardware_gpio FreeRTOS commands command_handler led input_interface)
pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)
pico_add_extra_outputs(main)

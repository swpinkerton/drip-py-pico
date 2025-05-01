# Add our executable
add_executable(sender sender.cpp)
add_executable(reciever reciever.cpp)


# Link to needed Pico libraries
target_link_libraries(sender pico_stdlib hardware_uart)
target_link_libraries(reciever pico_stdlib hardware_uart)


# Enable USB output / stdio
pico_enable_stdio_usb(sender 1)
pico_enable_stdio_uart(sender 0) # We are using UART manually

pico_enable_stdio_usb(reciever 1)
pico_enable_stdio_uart(reciever 0) # We are using UART manually

# Add other build types (like debug) here if needed

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(sender)
pico_add_extra_outputs(reciever)

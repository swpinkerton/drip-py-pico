cmake_minimum_required(VERSION 3.20)
project(pico-cpp LANGUAGES C CXX ASM)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
pico_sdk_init()

add_library(warnings INTERFACE)
target_compile_options(warnings INTERFACE "-Wall" "-Wextra" "-Wno-psabi")

add_subdirectory(src)

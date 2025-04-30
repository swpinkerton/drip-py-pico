cmake_minimum_required(VERSION 3.20)

include(pico_sdk_import.cmake)

project(pico-cpp LANGUAGES C CXX ASM)

pico_sdk_init()

add_compile_definitions(DEBUG_ENABLE)

add_library(warnings INTERFACE)
target_compile_options(warnings INTERFACE "-Wall" "-Wextra" "-Wno-psabi")

FILE(GLOB FreeRTOS_src FreeRTOS-Kernel/*.c)

add_library( FreeRTOS STATIC
	${FreeRTOS_src}
	FreeRTOS-Kernel/portable/GCC/ARM_CM0/port.c
	FreeRTOS-Kernel/portable/MemMang/heap_4.c
)

target_include_directories( FreeRTOS PUBLIC
		FreeRTOS-Kernel/include
		include/
		FreeRTOS-Kernel/portable/GCC/ARM_CM0
	)

add_subdirectory(src)
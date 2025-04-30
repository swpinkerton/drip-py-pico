#pragma once
#include "pico/stdlib.h"

// To enable/disable debug prints, add/remove "add_compile_definitions(DEBUG_ENABLE)" from the top level CMakeLists.txt file

#ifdef DEBUG_ENABLE
    #define DPRINTF(fmt, ...)   printf(fmt, ##__VA_ARGS__)
#else
    #define DPRINTF(fmt, ...)
#endif

void verbal_pause(uint seconds);
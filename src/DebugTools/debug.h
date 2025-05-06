#pragma once
#include "pico/stdlib.h"

/**
 * @brief Print only if DEBUG_ENABLE is defined
 * 
 * To enable/disable debug prints, add/remove "add_compile_definitions(DEBUG_ENABLE)" from the top level CMakeLists.txt file.
 */
#ifdef DEBUG_ENABLE
    #define DPRINTF(fmt, ...)   printf(fmt, ##__VA_ARGS__)
#else
    #define DPRINTF(fmt, ...)
#endif

/**
 * @brief Pause for number of seconds while counting
 * 
 * @param seconds 
 */
void verbal_pause(uint seconds);
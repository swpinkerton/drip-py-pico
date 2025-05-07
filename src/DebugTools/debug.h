#pragma once
#include "pico/stdlib.h"
#include "debug_config.h"

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
 * @brief Specific debug print macro for mutex debugging
 * 
 */
#ifdef DEBUG_MUTEX
    #define DPRINTF_MUTEX(fmt, ...)     DPRINTF(fmt, ##__VA_ARGS__)
#else
    #define DPRINTF_MUTEX(fmt, ...)
#endif

/**
 * @brief Specific debug print macro for trace debugging
 * 
 */
#ifdef DEBUG_TRACE
    #define DPRINTF_TRACE(fmt, ...)     DPRINTF(fmt, ##__VA_ARGS__)
#else
    #define DPRINTF_TRACE(fmt, ...)
#endif

/**
 * @brief Print function on entering
 * 
 * This macro will print the function when it is entered during run time.
 */
#define DTRACE()   DPRINTF_TRACE("%s at line: %d: %s()\n", __FILE__, __LINE__, __FUNCTION__)

/**
 * @brief Pause for number of seconds and print progress each second
 * 
 * @param seconds Number of seconds to wait
 */
void verbal_pause(uint seconds);
#include "debug.h"
#include <stdio.h>

#ifdef DEBUG
#include <stdio.h>
#include <stdarg.h>
#endif

void dprintf(const char *format, ...) {
#ifdef DEBUG
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
#endif
}

void verbal_pause(uint seconds) {
    for (int i = 0; i < seconds; i++) {
        printf("Waiting %d/%d\n", i, seconds);
        sleep_ms(1000);
    }
}
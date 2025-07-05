#include "debug.h"
#include <stdio.h>

void verbal_pause(uint seconds) {
    for (int i = 0; i < seconds; i++) {
        printf("Waiting %d/%d\n", i, seconds);
        sleep_ms(1000);
    }
}
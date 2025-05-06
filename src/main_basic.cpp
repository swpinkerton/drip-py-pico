#include "debug.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "gantry.h"
#include "dropper.h"
#include "controller.h"
#include "stepper.h"
#include <vector>


void core1_entry() {
    motor_control_loop();
}

int main() {
    stdio_init_all();

    printf("Core 0 Started...\n");

    init_gantry();
    init_dropper();

    multicore_launch_core1(core1_entry);

    while(1) {

        move_xy(1000,1000);
        sleep_ms(10000);
        move_xy(0,0);
        sleep_ms(10000);
    }   
}
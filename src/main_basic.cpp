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

    verbal_pause(3);

    printf("Core 0 Started...\n");

    // while(1) {
    //     DPRINTF_TYPE(BALLS, "balls2\n");
    // }

    init_gantry();
    init_dropper();

    multicore_launch_core1(core1_entry);

    printf("resetting gantry\n");
    reset_gantry();
    wait_on_gantry_reset();
    printf("reset complete\n");

    while(1) {
        printf("balls\n");
        goto_well(0,0);
        sleep_ms(3000);
        goto_well(1,0);
        sleep_ms(3000);
        goto_well(5,0);
        sleep_ms(3000);
        // move_xy(20, 20);
        // sleep_ms(6000);
        // move_xy(0,0);
        // sleep_ms(6000);
    }   
}
#define DEBUG
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "stepper.hpp"

void core1_entry() {
    motor_control_loop();
}

// void run_command(char command[]) {

// }

int main() {
    stdio_init_all();

    printf("Core 0 Started...\n");

    multicore_launch_core1(core1_entry);

    while(1) {
        printf("Core 0 hello\n");
        move_xy(40,40);
        sleep_ms(3000);
        move_xy(0,0);
        sleep_ms(3000);
    }
}
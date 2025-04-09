#include <stdio.h>

#include "hardware/gpio.h"
#include <pico/stdlib.h>
#include "stepper.hpp"


int main() {
    stdio_init_all();

    sleep_ms(5000);

    printf("Started\n");

    const uint STEP_PIN = 7;
    const uint DIR_PIN = 6;

    uint sleep_time = 50000;

    gpio_init(STEP_PIN);
    gpio_init(DIR_PIN);
    gpio_set_dir(STEP_PIN, GPIO_OUT);
    gpio_set_dir(DIR_PIN, GPIO_OUT);

    gpio_put(DIR_PIN, 1);
    while (true) {
        // printf("hello");
        gpio_put(STEP_PIN, 1);
        sleep_us(sleep_time);
        gpio_put(STEP_PIN, 0);
        sleep_us(sleep_time);
    }
    // motor_t x_motor = {0};
    // motor_t y_motor = {0};
    // stepper_motor_init(&x_motor, X_AXIS);
    // stepper_motor_init(&y_motor, Y_AXIS);

    // int i = 0;
    // int dir = -1;
    // while (1) {
    //     stepper_motor_step(&x_motor, dir);
    //     stepper_motor_step(&y_motor, dir);
        
    //     sleep_ms(2);
    //     i += 2;
    //     if (i >= 1000) {
    //         i = 0;
    //         dir = -dir;
    //         printf("dir %d", dir);
    //     }

    // }
}

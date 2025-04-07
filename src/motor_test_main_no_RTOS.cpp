#include <stdio.h>

#include "hardware/gpio.h"
#include <pico/stdlib.h>
#include "stepper.hpp"


int main() {
    stdio_init_all();

    sleep_ms(5000);

    printf("Started\n");
    motor_t x_motor = stepper_motor_basic_init(X_AXIS);
    motor_t y_motor = stepper_motor_basic_init(Y_AXIS);
    // motor_t motor = stepper_motor_init(X_AXIS);
    // printf("motor.busy: %d\n", motor.busy);
    int i = 0;
    int dir = -1;
    while (1) {
        stepper_motor_step(&x_motor, dir);
        stepper_motor_step(&y_motor, dir);
        sleep_ms(2);
        i += 2;
        if (i >= 1000) {
            i = 0;
            dir = -dir;
            printf("dir %d", dir);
        }

    }
}

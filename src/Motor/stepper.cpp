#include "stepper.h"
#include "motor_settings.h"
#include "stepper.h"
#include "pico/stdlib.h"
#include "hardware/irq.h"
#include <math.h>
#include <stdio.h>
#include <cstdlib>
#include "pico/multicore.h"

int8_t sign(int n) {
    DTRACE();
    if (n >= 0) return 1;
    else return -1;
}

void set_motor_target_rpm(uint steps, uint rpm, motor_t* motor) {
    LOCK_MOTOR(motor);
    motor->target_speed = steps*rpm/60;
    UNLOCK_MOTOR(motor);
}

void set_motor_acceleration(uint steps, uint rpm, motor_t* motor) {
    motor->acceleration = steps*rpm/60;
}

void stepper_motor_init(motor_t* motor, motor_pins_t pins) {
    DTRACE();
    // GPIO pins
    motor->pins = pins;

    // Init motor variables
    motor->direction = 1;
    motor->location = 0;
    motor->target = 0;
    motor->enabled = false;
    motor->next_step_time = 0;
    motor->target_speed = 0;
    motor->acceleration = 0;

    // Initialise the GPIO
    gpio_init(motor->pins.dir);
    gpio_init(motor->pins.step);
    gpio_init(motor->pins.enable);
    
    gpio_set_dir(motor->pins.dir, GPIO_OUT);
    gpio_set_dir(motor->pins.step, GPIO_OUT);
    gpio_set_dir(motor->pins.enable, GPIO_OUT);

    // Init the mutex
    mutex_init(&motor->lock);

    // Make sure motors are disabled on startup
    disable_motor(motor);
}

void enable_motor(motor_t* motor) {
    DTRACE();
    LOCK_MOTOR(motor);
    motor->enabled = true;
    UNLOCK_MOTOR(motor);
    gpio_put(motor->pins.enable, 0);
}

void disable_motor(motor_t* motor) {
    DTRACE();
    LOCK_MOTOR(motor);
    motor->enabled = false;
    UNLOCK_MOTOR(motor);
    gpio_put(motor->pins.enable, 1);
}

void set_motor_target(motor_t* motor, int target) {
    DTRACE();
    LOCK_MOTOR(motor);

    if (motor->target == target) {
        // Motor is already doing the right thing so change nothing.
        UNLOCK_MOTOR(motor);
        return;
    }

    motor->direction = (int8_t) sign(target - motor->location);
    motor->target = target;
    motor->next_step_time = to_us_since_boot(get_absolute_time());
    motor->move_start_time = to_us_since_boot(get_absolute_time());
    motor->step_counter = 0;
    
    // Find the number of steps for acceleration/deceleration
    uint acceleration_time = motor->target_speed/motor->acceleration;
    motor->steps_to_accel = motor->acceleration/2 * acceleration_time * acceleration_time;

    UNLOCK_MOTOR(motor);

    enable_motor(motor);
}

void set_motor_relative_target(motor_t* motor, int delta_steps) {
    DTRACE();
    LOCK_MOTOR(motor);

    if (delta_steps == 0) {
        // No move
        UNLOCK_MOTOR(motor);
        return;
    }

    int new_target = motor->target + delta_steps;

    if (new_target < 0) new_target = 0;

    motor->direction = (int8_t) sign(new_target - motor->location);
    motor->target = new_target;
    motor->next_step_time = to_us_since_boot(get_absolute_time());
    motor->move_start_time = to_us_since_boot(get_absolute_time());
    motor->step_counter = 0;
    
    // Find the number of steps for acceleration/deceleration
    uint acceleration_time = motor->target_speed/motor->acceleration;
    motor->steps_to_accel = motor->acceleration/2 * acceleration_time * acceleration_time;

    UNLOCK_MOTOR(motor);

    enable_motor(motor);
}
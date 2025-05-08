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

int mm_to_steps(float mm) {
    DTRACE();
    float x_revolutions = mm / THREAD_PITCH_MM;
    return static_cast<int>(x_revolutions * MOTOR_STEPS_PER_REVOLUTION * MICROSTEPS);
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
    motor->step_time = 250;
    motor->target_speed = MAX_SPEED;

    // Initialise the GPIO
    gpio_init(motor->pins.dir);
    gpio_init(motor->pins.step);
    gpio_init(motor->pins.enable);
    
    gpio_set_dir(motor->pins.dir, GPIO_OUT);
    gpio_set_dir(motor->pins.step, GPIO_OUT);
    gpio_set_dir(motor->pins.enable, GPIO_OUT);

    // Init the mutex
    mutex_init(&motor->lock);
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
    target = mm_to_steps(target);

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
    uint acceleration_time = motor->target_speed/ACCELERATION;
    motor->steps_to_accel = ACCELERATION/2 * acceleration_time * acceleration_time;

    UNLOCK_MOTOR(motor);

    enable_motor(motor);
}
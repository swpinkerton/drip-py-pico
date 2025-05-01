#include "stepper.hpp"
#include "debug.h"
#include "pico/time.h"
#include "pico/stdlib.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"
#include <math.h>
#include <stdio.h>
#include <cstdlib>
#include "pico/multicore.h"

// Global variables
static motor_t x_motor;
static motor_t y_motor;
static motor_t z_drop_motor;
static motor_t z_elec_motor;
static uint n_active_motors;

int8_t sign(int n) {
    if (n >= 0) return 1;
    else return -1;
}

void stepper_motor_init(motor_t* motor, axis_t axis) {
    // Setup the motor struct

    switch (axis)
    {
    case X_AXIS:
        motor->pin_dir       = X_DIR_PIN;
        motor->pin_step      = X_STEP_PIN;
        motor->pin_enable    = X_ENABLE_PIN;
        break;
    
    case Y_AXIS:
        motor->pin_dir       = Y_DIR_PIN;
        motor->pin_step      = Y_STEP_PIN;
        motor->pin_enable    = Y_ENABLE_PIN;
        break;
        
    case Z_DROPPER:
        motor->pin_dir       = DROP_DIR_PIN;
        motor->pin_step      = DROP_STEP_PIN;
        motor->pin_enable    = DROP_ENABLE_PIN;
        break;

    case Z_ELECTROSTIM:
        motor->pin_dir       = ELEC_DIR_PIN;
        motor->pin_step      = ELEC_STEP_PIN;
        motor->pin_enable    = ELEC_ENABLE_PIN;
        break;
    }

    // Init motor variables
    motor->direction = 1;
    motor->location = 0;
    motor->target = 0;
    motor->enabled = false;
    motor->next_step_time = 0;
    motor->step_time = 250;
    motor->target_speed = MAX_SPEED;

    // Initialise the GPIO
    gpio_init(motor->pin_dir);
    gpio_init(motor->pin_step);
    gpio_init(motor->pin_enable);
    
    gpio_set_dir(motor->pin_dir, GPIO_OUT);
    gpio_set_dir(motor->pin_step, GPIO_OUT);
    gpio_set_dir(motor->pin_enable, GPIO_OUT);

    // Init the mutex
    mutex_init(&motor->lock);
}

void enable_motor(motor_t* motor) {
    n_active_motors++;
    motor->enabled = true;
    gpio_put(motor->pin_enable, 1);
}

void disable_motor(motor_t* motor) {
    n_active_motors--;
    motor->enabled = false;
    gpio_put(motor->pin_enable, 0);
}

int mm_to_steps(float mm) {
    float x_revolutions = mm / THREAD_PITCH_MM;
    return (int) x_revolutions * MOTOR_STEPS_PER_REVOLUTION;
}

void zero_motor(motor_t* motor) {
    mutex_enter_blocking(&motor->lock);
    motor->target = 0;
    motor->direction = 1;
    motor->location = 0;
    motor->enabled = false;
    mutex_exit(&motor->lock);
}

void endstop_irq_handler(uint gpio) {
    switch (gpio)
    {
    case X_ENDSTOP_PIN:
        zero_motor(&x_motor);
        break;
    
    case Y_ENDSTOP_PIN:
        zero_motor(&y_motor);
        break;

    default:
        break;
    }
}

void set_motor_rpm(motor_t* motor, uint rpm) {
    rpm = (rpm > MAX_RPM) ? MAX_RPM : rpm;
    motor->target_speed = rpm * MOTOR_STEPS_PER_REVOLUTION * MICROSTEPS / 60;
}

void set_motor_target(motor_t* motor, int target) {
    target = mm_to_steps(target);

    mutex_enter_blocking(&motor->lock);
    motor->direction = (int8_t) sign(target - motor->location);
    motor->target = target;
    motor->next_step_time = to_us_since_boot(get_absolute_time());
    motor->move_start_time = to_us_since_boot(get_absolute_time());
    motor->step_counter = 0;
    
    // Find the number of steps for acceleration/deceleration
    uint acceleration_time = motor->target_speed/ACCELERATION;
    motor->steps_to_accel = ACCELERATION/2 * acceleration_time * acceleration_time;

    enable_motor(motor);
    mutex_exit(&motor->lock);
}

void move_xy(int x, int y) {
    set_motor_target(&x_motor, x);
    set_motor_target(&y_motor, y);
}

void reset_axis() {
    move_xy(-9999999, -9999999);
}

motor_state_t check_status() {
    if (n_active_motors == 0) {
        return STOPPED;
    } else {
        return MOVING;
    }
}

void motor_control_loop()
{  
    verbal_pause(6);
    
    // Initialise the motor gpios
    DPRINTF("Setting up motors\n");

    stepper_motor_init(&x_motor, X_AXIS);
    stepper_motor_init(&y_motor, Y_AXIS);
    stepper_motor_init(&z_drop_motor, Z_DROPPER);
    stepper_motor_init(&z_elec_motor, Z_ELECTROSTIM);

    motor_t* motors[2];
    motors[X_AXIS] = &x_motor;
    motors[Y_AXIS] = &y_motor;
    // motors[Z_DROPPER] = &z_drop_motor;
    // motors[Z_ELECTROSTIM] = &z_elec_motor;

    n_active_motors = 0;

    // Create the interrupts for handling the endstops.
    gpio_set_irq_enabled(X_ENDSTOP_PIN, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(Y_ENDSTOP_PIN, GPIO_IRQ_EDGE_FALL, true);
    gpio_pull_up(X_ENDSTOP_PIN);
    gpio_pull_up(Y_ENDSTOP_PIN);

    uint64_t time = 10;

    DPRINTF("Starting Motor Control Loop\n");

    gpio_init(21);
    gpio_set_dir(21, GPIO_OUT);
    gpio_put(21, 0);

    while (1) {
        // if time elapsed < motor wait time step motor, calc next step time.
        time = to_us_since_boot(get_absolute_time());

        // Check direction pins are correct
        for (motor_t* motor : motors) {
            mutex_enter_blocking(&motor->lock);
            int8_t pin_state = gpio_get(motor->pin_dir);
            if (pin_state == 0) pin_state = -1;

            if (pin_state != motor->direction) {
                int8_t new_dir = motor->direction;
                if (new_dir == -1) new_dir = 0;
                gpio_put(motor->pin_dir, new_dir);
            }
            mutex_exit(&motor->lock);
        }

        // Set all step pins
        for (motor_t* motor : motors) {
            mutex_enter_blocking(&motor->lock);
            if (motor->enabled) {
                if (time > motor->next_step_time) {
                    DPRINTF("Time: %llu\n", time);
                    gpio_put(motor->pin_step, 1);

                    DPRINTF("Steps to accell %d\n", motor->steps_to_accel);
                    DPRINTF("step counter %d\n", motor->step_counter);

                    // Calculate the next step time.
                    // Increment step counter now as we want to find time of the next step.
                    motor->step_counter++;

                    // Check if we are in acceleration
                    if (motor->step_counter <= motor->steps_to_accel) {
                        // Use quadratic eq to find time we want to be at next step.
                        // distance = acceleration/2 * time^2
                        DPRINTF("accelerating...\n");
                        uint next_step_time = sqrt(motor->step_counter/(ACCELERATION/2.0)) * 1e6;
                        DPRINTF("Move start time %d\n", motor->move_start_time);
                        motor->next_step_time = motor->move_start_time + next_step_time;
                    } else {//if (motor->step_counter < motor->total_steps_in_move - motor->steps_to_accel) {
                        // We are in constant speed section and have reached max speed.
                        motor->next_step_time += 1e6/motor->target_speed;
                    }

                    motor->location += motor->direction;
                    DPRINTF("stepping motor . T: %d, L: %d, D: %d, NST: %llu\n", motor->target, motor->location, motor->direction, motor->next_step_time);
                    
                    // If motor has reached target, disable it.
                    if (motor->location == motor->target) {
                        disable_motor(motor);
                    }
                }
            }

            mutex_exit(&motor->lock);
        }
        sleep_us(1);

        // Unset all motor step pins.
        for (motor_t* motor : motors) {
            mutex_enter_blocking(&motor->lock);
            gpio_put(motor->pin_step, 0);
            mutex_exit(&motor->lock);
        }
        sleep_us(1);
    }
}
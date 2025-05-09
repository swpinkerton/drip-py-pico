#include "controller.h"
#include <vector>
#include "stepper.h"
#include "debug.h"
#include "pico/multicore.h"
#include "motor_settings.h"
#include <math.h>
#include "pico/stdlib.h"
#include <stdio.h>

using std::vector;

vector<motor_t*> motor_list;

void manual_step(int num_steps) {
    return;
}

void add_motor(motor_t* motor) {
    DTRACE();
    motor_list.push_back(motor);
}

void stop_motors() {
    DTRACE();
    for (motor_t* motor : motor_list) {
        disable_motor(motor);
        LOCK_MOTOR(motor);
        motor->target = motor->location;
        UNLOCK_MOTOR(motor);
    }
}

void motor_control_loop() {  
    DTRACE();

    uint64_t time = 0;

    DPRINTF_TYPE(DEBUG_MOTOR, "Starting Motor Control Loop\n");

    while (1) {
        // if time elapsed < motor wait time step motor, calc next step time.
        time = to_us_since_boot(get_absolute_time());

        // Check direction pins are correct
        for (motor_t* motor : motor_list) {
            LOCK_MOTOR(motor);
            int8_t pin_state = gpio_get(motor->pins.dir);
            if (pin_state == 0) pin_state = -1;

            if (pin_state != motor->direction) {
                int8_t new_dir = motor->direction;
                if (new_dir == -1) new_dir = 0;
                gpio_put(motor->pins.dir, new_dir);
            }
            UNLOCK_MOTOR(motor);
        }

        // Set all step pins
        for (motor_t* motor : motor_list) {
            LOCK_MOTOR(motor);
            if (motor->enabled) {
                // Safety check if motor is already at target, disable it.
                if (motor->location == motor->target) {
                    UNLOCK_MOTOR(motor);
                    disable_motor(motor);
                    LOCK_MOTOR(motor);
                } 
                else if (time > motor->next_step_time) {
                    DPRINTF_TYPE(DEBUG_MOTOR, "Time: %llu\n", time);
                    gpio_put(motor->pins.step, 1);

                    DPRINTF_TYPE(DEBUG_MOTOR, "Steps to accell %d\n", motor->steps_to_accel);
                    DPRINTF_TYPE(DEBUG_MOTOR, "step counter %d\n", motor->step_counter);

                    // Calculate the next step time.
                    // Increment step counter now as we want to find time of the next step.
                    motor->step_counter++;

                    // Check if we are in acceleration
                    if (motor->step_counter <= motor->steps_to_accel) {
                        // Use quadratic eq to find time we want to be at next step.
                        // distance = acceleration/2 * time^2
                        DPRINTF_TYPE(DEBUG_MOTOR, "accelerating...\n");
                        uint next_step_time = sqrt(motor->step_counter/(motor->acceleration/2.0)) * 1e6;
                        DPRINTF_TYPE(DEBUG_MOTOR, "Move start time %d\n", motor->move_start_time);
                        motor->next_step_time = motor->move_start_time + next_step_time;
                    } else {//if (motor->step_counter < motor->total_steps_in_move - motor->steps_to_accel) {
                        // We are in constant speed section and have reached max speed.
                        motor->next_step_time += 1e6/motor->target_speed;
                    }

                    motor->location += motor->direction;
                    DPRINTF_TYPE(DEBUG_MOTOR, "stepping motor . T: %d, L: %d, D: %d, NST: %llu\n", motor->target, motor->location, motor->direction, motor->next_step_time);
                    
                    // If motor has reached target, disable it.
                    if (motor->location == motor->target) {
                        UNLOCK_MOTOR(motor);
                        disable_motor(motor);
                        LOCK_MOTOR(motor);
                    }
                }
            }

            UNLOCK_MOTOR(motor);
        }
        sleep_us(1);

        // Unset all motor step pins.
        for (motor_t* motor : motor_list) {
            LOCK_MOTOR(motor);
            gpio_put(motor->pins.step, 0);
            UNLOCK_MOTOR(motor);
        }
        sleep_us(1);
    }
}
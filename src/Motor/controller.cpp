#include "controller.h"
#include <vector>
#include "stepper.h"
#include "debug.h"
#include "pico/multicore.h"
#include "motor_settings.h"
#include <math.h>

using std::vector;

vector<motor_t*> motor_list;

void add_motor(motor_t* motor) {
    motor_list.push_back(motor);
}

void motor_control_loop() {  

    uint64_t time = 0;

    DPRINTF("Starting Motor Control Loop\n");

    while (1) {
        // if time elapsed < motor wait time step motor, calc next step time.
        time = to_us_since_boot(get_absolute_time());

        // Check direction pins are correct
        for (motor_t* motor : motor_list) {
            mutex_enter_blocking(&motor->lock);
            int8_t pin_state = gpio_get(motor->pins.dir);
            if (pin_state == 0) pin_state = -1;

            if (pin_state != motor->direction) {
                int8_t new_dir = motor->direction;
                if (new_dir == -1) new_dir = 0;
                gpio_put(motor->pins.dir, new_dir);
            }
            mutex_exit(&motor->lock);
        }

        // Set all step pins
        for (motor_t* motor : motor_list) {
            mutex_enter_blocking(&motor->lock);
            if (motor->enabled) {
                if (time > motor->next_step_time) {
                    DPRINTF("Time: %llu\n", time);
                    gpio_put(motor->pins.step, 1);

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
        for (motor_t* motor : motor_list) {
            mutex_enter_blocking(&motor->lock);
            gpio_put(motor->pins.step, 0);
            mutex_exit(&motor->lock);
        }
        sleep_us(1);
    }
}
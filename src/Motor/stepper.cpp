#include "stepper.hpp"
#include "pico/time.h"
#include "pico/stdlib.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"
#include <stdio.h>
#include "FreeRTOS.h"
#include "queue.h"
#include <cstdlib>

static bool going_up = false;
static int pwm_level = 0;
static int pwm_increment = 1;

int8_t sign(int n) {
    if (n >= 0) return 1;
    else return -1;
}

bool motor_irq_handler(repeating_timer_t *rt) {

// #ifdef DEBUG
    printf("ISR entered\n");
// #endif

    motor_t* motor_data = (motor_t*) rt->user_data;

    if (going_up) {
        pwm_level += pwm_increment;
        if (pwm_level > PWM_WRAP) {
            pwm_level = PWM_WRAP;

            cancel_repeating_timer(rt);
            // motor_data->busy = false;
            pwm_set_enabled(pwm_gpio_to_slice_num(motor_data->pin_apwm), false);
            pwm_set_enabled(pwm_gpio_to_slice_num(motor_data->pin_bpwm), false);
        }
    } else {
        pwm_level -= pwm_increment;
        if (pwm_level < 0) {
            pwm_level = 0;

            cancel_repeating_timer(rt);
            // motor_data->busy = false;
            pwm_set_enabled(pwm_gpio_to_slice_num(motor_data->pin_apwm), false);
            pwm_set_enabled(pwm_gpio_to_slice_num(motor_data->pin_bpwm), false);
        }
    }

// #ifdef DEBUG
    printf("a level: %d\n", pwm_level);
    printf("b level: %d\n", PWM_WRAP-pwm_level);
// #endif

    pwm_set_gpio_level(motor_data->pin_apwm, pwm_level);
    pwm_set_gpio_level(motor_data->pin_bpwm, PWM_WRAP-pwm_level);

    return true;
}

motor_t stepper_motor_init(axis_t axis) {

    // Setup the motor struct
    motor_t motor;

    motor.step_cycle = 0;

    switch (axis)
    {
    case X_AXIS:
        motor.pin_a1    = X_AIN_1;
        motor.pin_a2    = X_AIN_2;
        motor.pin_apwm  = X_A_PWM;
        motor.pin_b1    = X_BIN_1;
        motor.pin_b2    = X_BIN_2;
        motor.pin_bpwm  = X_B_PWM;
        break;
    
    case Y_AXIS:
        motor.pin_a1    = Y_AIN_1;
        motor.pin_a2    = Y_AIN_2;
        motor.pin_apwm  = Y_A_PWM;
        motor.pin_b1    = Y_BIN_1;
        motor.pin_b2    = Y_BIN_2;
        motor.pin_bpwm  = Y_B_PWM;
        break;
    }

    // Initialise the GPIO and PWM hardware

    gpio_init(motor.pin_a1);
    gpio_init(motor.pin_a2);
    gpio_init(motor.pin_b1);
    gpio_init(motor.pin_b2);
    
    gpio_set_dir(motor.pin_a1, GPIO_OUT);
    gpio_set_dir(motor.pin_a2, GPIO_OUT);
    gpio_set_dir(motor.pin_b1, GPIO_OUT);
    gpio_set_dir(motor.pin_b2, GPIO_OUT);

    gpio_set_function(motor.pin_apwm, GPIO_FUNC_PWM);
    gpio_set_function(motor.pin_bpwm, GPIO_FUNC_PWM);

    uint slice_num_a = pwm_gpio_to_slice_num(motor.pin_apwm);
    uint slice_num_b = pwm_gpio_to_slice_num(motor.pin_bpwm);

    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 1.f);
    pwm_config_set_wrap(&config, PWM_WRAP);
    pwm_init(slice_num_a, &config, false);
    pwm_init(slice_num_b, &config, false);

    return motor;
}

motor_t stepper_motor_basic_init(axis_t axis) {
    // Setup the motor struct
    motor_t motor;

    motor.step_cycle = 0;

    switch (axis)
    {
    case X_AXIS:
        motor.pin_a1    = X_AIN_1;
        motor.pin_a2    = X_AIN_2;
        motor.pin_apwm  = X_A_PWM;
        motor.pin_b1    = X_BIN_1;
        motor.pin_b2    = X_BIN_2;
        motor.pin_bpwm  = X_B_PWM;
        break;
    
    case Y_AXIS:
        motor.pin_a1    = Y_AIN_1;
        motor.pin_a2    = Y_AIN_2;
        motor.pin_apwm  = Y_A_PWM;
        motor.pin_b1    = Y_BIN_1;
        motor.pin_b2    = Y_BIN_2;
        motor.pin_bpwm  = Y_B_PWM;
        break;
    }

    // Initialise the GPIO and PWM hardware

    gpio_init(motor.pin_a1);
    gpio_init(motor.pin_a2);
    gpio_init(motor.pin_apwm);
    gpio_init(motor.pin_b1);
    gpio_init(motor.pin_b2);
    gpio_init(motor.pin_bpwm);
    
    gpio_set_dir(motor.pin_a1, GPIO_OUT);
    gpio_set_dir(motor.pin_a2, GPIO_OUT);
    gpio_set_dir(motor.pin_apwm, GPIO_OUT);
    gpio_set_dir(motor.pin_b1, GPIO_OUT);
    gpio_set_dir(motor.pin_b2, GPIO_OUT);
    gpio_set_dir(motor.pin_bpwm, GPIO_OUT);

    return motor;
}

void pwm_check(motor_t* motor) {
    pwm_set_enabled(pwm_gpio_to_slice_num(motor->pin_apwm), true);
    pwm_set_enabled(pwm_gpio_to_slice_num(motor->pin_bpwm), true);

    pwm_set_gpio_level(motor->pin_apwm, PWM_WRAP/2);
    pwm_set_gpio_level(motor->pin_bpwm, PWM_WRAP/10);
}

void stepper_motor_step(motor_t* motor, int8_t direction) {

    gpio_put(motor->pin_apwm, 1);
    gpio_put(motor->pin_bpwm, 1);
    
    if (motor->step_cycle == 0) {
        gpio_put(motor->pin_a1, 1);
        gpio_put(motor->pin_a2, 0);
        gpio_put(motor->pin_b1, 0);
        gpio_put(motor->pin_b2, 1);
    } else if (motor->step_cycle == 1) {
        gpio_put(motor->pin_a1, 1);
        gpio_put(motor->pin_a2, 0);
        gpio_put(motor->pin_b1, 1);
        gpio_put(motor->pin_b2, 0);
    } else if (motor->step_cycle == 2) {
        gpio_put(motor->pin_a1, 0);
        gpio_put(motor->pin_a2, 1);
        gpio_put(motor->pin_b1, 1);
        gpio_put(motor->pin_b2, 0);
    } else if (motor->step_cycle == 3) {
        gpio_put(motor->pin_a1, 0);
        gpio_put(motor->pin_a2, 1);
        gpio_put(motor->pin_b1, 0);
        gpio_put(motor->pin_b2, 1);
    }

    // Update the step cycle

    if (direction == 1) {
        motor->step_cycle++;
        if (motor->step_cycle == 4) motor->step_cycle = 0;
    } else if (direction == -1) {
        if (motor->step_cycle == 0) motor->step_cycle = 3;
        else motor->step_cycle--;
    }
}

void stepper_motor_smooth_step(motor_t* motor, int8_t direction, uint step_time_ms, uint n_microsteps) {
    
    // if (motor->busy) {
    //     return;
    // }


// #ifdef DEBUG
    printf("smooth_step_function\n");
// #endif

    // motor->busy = true;

    pwm_set_enabled(pwm_gpio_to_slice_num(motor->pin_apwm), true);
    pwm_set_enabled(pwm_gpio_to_slice_num(motor->pin_bpwm), true);

    if (motor->step_cycle == 0) {
        gpio_put(motor->pin_a1, 1);
        gpio_put(motor->pin_a2, 0);
        gpio_put(motor->pin_b1, 0);
        gpio_put(motor->pin_b2, 1);
        pwm_level = 0;
        going_up = true;
    } else if (motor->step_cycle == 1) {
        gpio_put(motor->pin_a1, 1);
        gpio_put(motor->pin_a2, 0);
        gpio_put(motor->pin_b1, 1);
        gpio_put(motor->pin_b2, 0);
        pwm_level = PWM_WRAP;
        going_up = false;
    } else if (motor->step_cycle == 2) {
        gpio_put(motor->pin_a1, 0);
        gpio_put(motor->pin_a2, 1);
        gpio_put(motor->pin_b1, 1);
        gpio_put(motor->pin_b2, 0);
        pwm_level = 0;
        going_up = true;
    } else if (motor->step_cycle == 3) {
        gpio_put(motor->pin_a1, 0);
        gpio_put(motor->pin_a2, 1);
        gpio_put(motor->pin_b1, 0);
        gpio_put(motor->pin_b2, 1);
        pwm_level = PWM_WRAP;
        going_up = false;
    }

    // Create a repeating timer to handle increasing the PWM duty cycle
    repeating_timer_t timer;
    int64_t delay_us = step_time_ms * 1000 / n_microsteps;
    pwm_increment = PWM_WRAP/n_microsteps;
    add_repeating_timer_us(delay_us, motor_irq_handler, (void*) motor, &timer);

// #ifdef DEBUG
    printf("timer created\n");
// #endif

    // Update the step cycle

    if (direction == 1) {
        motor->step_cycle++;
        if (motor->step_cycle == 4) motor->step_cycle = 0;
    } else if (direction == -1) {
        if (motor->step_cycle == 0) motor->step_cycle = 3;
        else motor->step_cycle--;
    }
}

void disable_motor(motor_t* motor) {
    gpio_put(motor->pin_a1, 0);
    gpio_put(motor->pin_a2, 0);
    gpio_put(motor->pin_apwm, 0);
    gpio_put(motor->pin_b1, 0);
    gpio_put(motor->pin_b2, 0);
    gpio_put(motor->pin_bpwm, 0);
}

int mm_to_steps(float mm) {
    float x_revolutions = mm / THREAD_PITCH_MM;
    return (int) x_revolutions * MOTOR_STEPS_PER_REVOLUTION;
}

void motor_control_loop(QueueHandle_t command_queue, QueueHandle_t response_queue)
{
    // Initialise the motor gpios

    printf("Starting Motor Control Loop\n");

    motor_t x_motor = stepper_motor_basic_init(X_AXIS);
    motor_t y_motor = stepper_motor_basic_init(Y_AXIS);

    int x_delta_steps = 0;
    int y_delta_steps = 0;

    int8_t x_direction = 1;
    int8_t y_direction = 1;

    int x_location = 0;
    int y_location = 0;

    int x_target = 0;
    int y_target = 0;

    bool x_moving = false;
    bool y_moving = false;

    uint step_time = STEP_TIME_MS;
    uint n_microsteps = N_MICROSTEPS;

    TickType_t timeout = pdMS_TO_TICKS(10);
    BaseType_t xStatus;

    bool response_sent = true;

    motor_command_packet_t command;

    bool delay = false;

    while (1) {


        printf("Motor Status: Δx: %d, Δy: %d, xD: %d, yD: %d, xT: %d, yT: %d, xL: %d, yL: %d\n", x_delta_steps, y_delta_steps, x_direction, y_direction, x_target, y_target, x_location, y_location);
        
        // Check for command. If motors are moving, use timeout of 0.
        timeout = (x_moving or y_moving) ? 0 : 100;
        xStatus = xQueueReceive(command_queue, &command, timeout);

        // Process command.
        if (xStatus == pdPASS) {
            printf("Command Received\n");
            if (command.command == MOVE) {
                // If MOVE command, calculate new delta steps.
                if (command.axis == X_AXIS) {
                    x_target = mm_to_steps(command.target);
                    x_delta_steps = x_target - x_location;
                    x_direction = sign(x_delta_steps);
                    x_delta_steps = abs(x_delta_steps);
                    x_moving = true;
                }
                else {
                    y_target = mm_to_steps(command.target);
                    y_delta_steps = y_target - y_location;
                    y_direction = sign(y_delta_steps);
                    y_delta_steps = abs(y_delta_steps);
                    y_moving = true;
                }
            }
            response_sent = false;
        }

        // Move the motors if needed.
        // First check they aren't already mid-step.
        if (x_delta_steps != 0 and x_motor.busy == false) {
            // stepper_motor_smooth_step(&x_motor, x_direction, step_time, n_microsteps);
            stepper_motor_step(&x_motor, x_direction);
            x_delta_steps--;
            x_location += x_direction;
            delay = true;
        } else {
            x_moving = false;
            disable_motor(&x_motor);
        }

        if (y_delta_steps != 0 and y_motor.busy == false) {
            // stepper_motor_smooth_step(&y_motor, y_direction, step_time, n_microsteps);
            stepper_motor_step(&y_motor, y_direction);
            y_delta_steps--;
            y_location += y_direction;
            delay = true;
        } else {
            y_moving = false;
            disable_motor(&y_motor);
        }

        if (delay) {
            vTaskDelay(pdMS_TO_TICKS(step_time));
            delay = false;
        }

        // if no delta, send finished message.
        if (!x_moving and !y_moving and !response_sent) {
            const motor_response_t resp = COMPLETE;
            xStatus = xQueueSend(response_queue, (void*) &resp, 0);
            if (xStatus == pdPASS) {
                response_sent = true;
            }
        }
    }
}
#include "stepper_control.h"
#include "pico/time.h"
#include "pico/stdlib.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"
#include <stdio.h>

static bool going_up = false;
static int pwm_level = 0;
static int pwm_increment = 1;

bool motor_irq_handler(repeating_timer_t *rt) {

    printf("ISR entered\n");

    motor_t* motor_data = (motor_t*) rt->user_data;

    if (going_up) {
        pwm_level += pwm_increment;
        if (pwm_level > PWM_WRAP) {
            pwm_level = PWM_WRAP;

            cancel_repeating_timer(rt);
            motor_data->busy = false;
            pwm_set_enabled(pwm_gpio_to_slice_num(motor_data->pin_apwm), false);
            pwm_set_enabled(pwm_gpio_to_slice_num(motor_data->pin_bpwm), false);
        }
    } else {
        pwm_level -= pwm_increment;
        if (pwm_level < 0) {
            pwm_level = 0;

            cancel_repeating_timer(rt);
            motor_data->busy = false;
            pwm_set_enabled(pwm_gpio_to_slice_num(motor_data->pin_apwm), false);
            pwm_set_enabled(pwm_gpio_to_slice_num(motor_data->pin_bpwm), false);
        }
    }

    printf("a level: %d\n", pwm_level);
    printf("b level: %d\n", PWM_WRAP-pwm_level);

    pwm_set_gpio_level(motor_data->pin_apwm, pwm_level);
    pwm_set_gpio_level(motor_data->pin_bpwm, PWM_WRAP-pwm_level);

    return true;
}

motor_t stepper_motor_init(axis_t axis) {

    // Setup the motor struct
    motor_t motor;

    motor.step_cycle = 0;
    motor.step_incomplete = false;
    motor.n_microsteps = 10;
    motor.step_time_ms = 10;

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
    motor.step_incomplete = false;
    motor.n_microsteps = 10;
    motor.step_time_ms = 10;

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

void stepper_motor_step(motor_t* motor, motor_direction_t direction) {

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

    if (direction == CW) {
        motor->step_cycle++;
        if (motor->step_cycle == 4) motor->step_cycle = 0;
    } else if (direction == CCW) {
        motor->step_cycle--;
        if (motor->step_cycle == -1) motor->step_cycle = 3;
    }
}

bool stepper_motor_smooth_step(motor_t* motor, motor_direction_t direction, uint step_time_ms, uint n_microsteps) {
    
    if (motor->busy) {
        return false;
    } else {
        motor->busy = true;
    }

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

    printf("timer created\n");

    // Update the step cycle

    if (direction == CW) {
        motor->step_cycle++;
        if (motor->step_cycle == 4) motor->step_cycle = 0;
    } else if (direction == CCW) {
        motor->step_cycle--;
        if (motor->step_cycle == -1) motor->step_cycle = 3;
    }
}

void move_distance_xy(motor_t* x_motor, motor_t* y_motor, distance_t x_distance, distance_t y_distance) {

    // Calculate the number of steps for each motor
    uint x_revolutions = x_distance / THREAD_PITCH_MM;
    uint x_total_steps = x_revolutions * MOTOR_STEPS_PER_REVOLUTION;

    uint y_revolutions = y_distance / THREAD_PITCH_MM;
    uint y_total_steps = y_revolutions * MOTOR_STEPS_PER_REVOLUTION;

    // Move the motors
    uint x_completed_steps = 0;
    uint y_completed_steps = 0;

    bool x_finished = false;
    bool y_finished = false;

    uint step_time = MIN_STEP_TIME_MS;

    while (1) {

        if (!x_motor->busy && !x_finished) {

            stepper_motor_smooth_step(x_motor, CW, step_time, N_MICROSTEPS);
            x_completed_steps++;

            if (x_completed_steps >= x_total_steps) {
                x_finished = true;
            }
        }

        if (!x_motor->busy && !y_finished) {

            stepper_motor_smooth_step(y_motor, CW, step_time, N_MICROSTEPS);
            y_completed_steps++;
            
            if (y_completed_steps >= y_total_steps) {
                y_finished = true;
            }
        }

        if (x_finished && y_finished) {
            break;
        }
    }
    return;
}



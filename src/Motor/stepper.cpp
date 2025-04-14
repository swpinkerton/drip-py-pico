#include "stepper.hpp"
#include "pico/time.h"
#include "pico/stdlib.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"
#include <stdio.h>
#include "FreeRTOS.h"
#include "queue.h"
#include <cstdlib>

#ifdef DEBUG
#include <stdarg.h>

// Global variables
static motor_t x_motor;
static motor_t y_motor;
static motor_t z_drop_motor;
static motor_t z_elec_motor;

// Functions
void dprintf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}
#else
void dprintf(...) {}
#endif

int8_t sign(int n) {
    if (n >= 0) return 1;
    else return -1;
}

void stepper_motor_init(motor_t* motor, axis_t axis) {
    // Setup the motor struct

    switch (axis)
    {
    case X_AXIS:
        motor->pin_dir       = DROP_DIR_PIN;
        motor->pin_step      = DROP_STEP_PIN;
        motor->pin_enable    = DROP_ENABLE_PIN;
        break;
    
    case Y_AXIS:
        motor->pin_dir       = DROP_DIR_PIN;
        motor->pin_step      = DROP_STEP_PIN;
        motor->pin_enable    = DROP_ENABLE_PIN;
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

    // Initialise the GPIO

    gpio_init(motor->pin_dir);
    gpio_init(motor->pin_step);
    gpio_init(motor->pin_enable);
    
    gpio_set_dir(motor->pin_dir, GPIO_OUT);
    gpio_set_dir(motor->pin_step, GPIO_OUT);
    gpio_set_dir(motor->pin_enable, GPIO_OUT);
}

void stepper_motor_step(motor_t* motor, int8_t direction) {
    if (direction == 1) {
        gpio_put(motor->pin_dir, 1);
    } else {
        gpio_put(motor->pin_dir, 0);
    }
    // Delay 1us so direction pin setup time is met
    sleep_us(1);
    gpio_put(motor->pin_step, 1);
    // Delay 2us so step pin hold time is met
    sleep_us(2);
    gpio_put(motor->pin_step, 0);
}

void enable_motor(motor_t* motor) {
    motor->enabled = true;
    gpio_put(motor->pin_enable, 1);
}

void disable_motor(motor_t* motor) {
    motor->enabled = false;
    gpio_put(motor->pin_enable, 0);
}

int mm_to_steps(float mm) {
    float x_revolutions = mm / THREAD_PITCH_MM;
    return (int) x_revolutions * MOTOR_STEPS_PER_REVOLUTION;
}

void zero_axis(motor_t* motor) {
    motor->location = 0;
    motor->target = 0;
    motor->delta_steps = 0;
}

void calculate_targets(motor_t* motor, uint target) {
    target = mm_to_steps(target);
    printf("step target %d\n", target);
    int delta_steps =  target - motor->location;
    motor->delta_steps = (uint) abs(delta_steps);
    motor->direction = (int8_t) sign(delta_steps);
    motor->target = target;
}

void endstop_irq_handler(uint gpio) {
    switch (gpio)
    {
    case X_ENDSTOP_PIN:
        zero_axis(&x_motor);
        break;
    
    case Y_ENDSTOP_PIN:
        zero_axis(&y_motor);
        break;

    default:
        break;
    }
}

void motor_control_loop(QueueHandle_t command_queue, QueueHandle_t response_queue)
{
    // Initialise the motor gpios

    dprintf("Starting Motor Control Loop\n");

    stepper_motor_init(&x_motor, X_AXIS);
    stepper_motor_init(&y_motor, Y_AXIS);
    stepper_motor_init(&z_drop_motor, Z_DROPPER);
    stepper_motor_init(&z_elec_motor, Z_ELECTROSTIM);

    motor_t* motors[4];
    motors[X_AXIS] = &x_motor;
    motors[Y_AXIS] = &y_motor;
    motors[Z_DROPPER] = &z_drop_motor;
    motors[Z_ELECTROSTIM] = &z_elec_motor;

    // Create the interrupts for handling the endstops.
    gpio_set_irq_enabled(X_ENDSTOP_PIN, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(Y_ENDSTOP_PIN, GPIO_IRQ_EDGE_FALL, true);
    gpio_pull_up(X_ENDSTOP_PIN);
    gpio_pull_up(Y_ENDSTOP_PIN);

    uint step_time = STEP_TIME_MS;
    uint n_microsteps = N_MICROSTEPS;

    TickType_t timeout = pdMS_TO_TICKS(10);
    BaseType_t xStatus;

    bool response_sent = true;

    motor_command_packet_t command;

    bool delay = false;

    uint8_t motors_moving = 0;

    while (1) {

#ifdef DEBUG
        for (motor_t* motor : motors) {
            printf("Δ: %d, dir: %d, T: %d, L: %d\n", motor->delta_steps, motor->direction, motor->target, motor->location);
        }
#endif
        
        // Check for command. If motors are moving, use timeout of 0.
        timeout = motors_moving ? 0 : 100;
        xStatus = xQueueReceive(command_queue, &command, timeout);

        // Process command.
        if (xStatus == pdPASS) {
            dprintf("Command Received\n");

            // Move command
            if (command.command == MOVE) {
                printf("command.target %d\n", command.target);
                motor_t* target_motor = motors[command.axis];
                calculate_targets(target_motor, command.target);
                enable_motor(target_motor);
                motors_moving++;
            } 
            // Zero command
            else if (command.command == ZERO) {
                for (motor_t* motor : motors) {
                    // Go backwards until endstops are hit.
                    motor->delta_steps = -99999999;
                    enable_motor(motor);
                    motors_moving++;
                };
            }
            
            response_sent = false;
        }

        for (motor_t* motor : motors) {
            if (motor->enabled) {
                // Check is motor has arrived before stepping to prevent missing target
                if (motor->delta_steps == 0) {
                    disable_motor(motor);
                    motors_moving--;
                }
                stepper_motor_step(motor, motor->direction);
                motor->delta_steps -= motor->direction;
                motor->location += motor->direction;
            }
        }

        // if no delta, send finished message.
        if (motors_moving == 0 and !response_sent) {
            const motor_response_t resp = COMPLETE;
            xStatus = xQueueSend(response_queue, (void*) &resp, 0);
            if (xStatus == pdPASS) {
                response_sent = true;
            }
        }
    }
}
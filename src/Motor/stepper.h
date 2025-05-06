#pragma once
#include "pico/stdlib.h"
#include "pico/multicore.h"

struct motor_pins_t{
    uint step;
    uint dir;
    uint enable;
};

struct motor_t {
    // Pins
    motor_pins_t pins;

    // Motion
    uint location;
    uint target;
    int8_t direction;
    int target_speed;
    uint steps_to_accel;
    uint steps_in_move;

    // Vars for linear acceleration
    uint step_counter;
    uint move_start_time;
    uint total_steps_in_move;

    uint64_t next_step_time;
    uint step_time;

    bool enabled;

    // Sync
    mutex_t lock;
};

void disable_motor(motor_t* motor);

void enable_motor(motor_t* motor);

void set_motor_target(motor_t* motor, int target);

void stepper_motor_init(motor_t* motor, motor_pins_t pins);
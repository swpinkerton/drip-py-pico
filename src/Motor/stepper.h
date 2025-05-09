#pragma once
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "debug.h"

/**
 * @brief Macro for locking the motor mutexs
 * 
 */
#define LOCK_MOTOR(motor)       \
    do {\
        DPRINTF_MUTEX("Waiting on %p\n", motor);\
        mutex_enter_blocking(&motor->lock);\
        DPRINTF_MUTEX("Locking %p\n", motor);\
    } while (0)

/**
 * @brief Macro for unlocking motor mutexs
 * 
 */
#define UNLOCK_MOTOR(motor)     \
    do {\
        mutex_exit(&motor->lock);\
        DPRINTF_MUTEX("Unlocking %p\n", motor);\
    } while(0)

/**
 * @brief Struct to hold motor GPIO pins
 * 
 */
struct motor_pins_t{
    uint step;
    uint dir;
    uint enable;
};

/**
 * @brief Struct to hold motor related variables.
 * 
 */
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

/**
 * @brief Disable power to the motor
 * 
 * @param motor 
 */
void disable_motor(motor_t* motor);

/**
 * @brief Enable power to the motor
 * 
 * @param motor 
 */
void enable_motor(motor_t* motor);

/**
 * @brief Set the target location for the motors in steps
 * 
 * @param motor 
 * @param target Target location in steps
 */
void set_motor_target(motor_t* motor, int target);

/**
 * @brief Initiliase the motor GPIO and variables
 * 
 * @param motor 
 * @param pins 
 */
void stepper_motor_init(motor_t* motor, motor_pins_t pins);
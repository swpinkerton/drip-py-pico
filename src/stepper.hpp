#include "pico/stdlib.h"


// X Motor
#define X_AIN_1   4
#define X_AIN_2   3
#define X_A_PWM   2
#define X_BIN_1   6
#define X_BIN_2   7
#define X_B_PWM   8

// Y Motor
#define Y_AIN_1   3
#define Y_AIN_2   4
#define Y_A_PWM   2
#define Y_BIN_1   6
#define Y_BIN_2   7
#define Y_B_PWM   8

// Physical Parameters
#define THREAD_PITCH_MM             0.4
#define MOTOR_STEPS_PER_REVOLUTION  24
#define MIN_STEP_TIME_MS            2

// PWM Settings
#define PWM_WRAP        1024
#define N_MICROSTEPS    20


typedef struct {
    uint step_cycle;
    bool step_incomplete;

    uint pin_a1;
    uint pin_a2;
    uint pin_apwm;

    uint pin_b1;
    uint pin_b2;
    uint pin_bpwm;

    uint step_time_ms;
    uint n_microsteps;

    bool busy;
} motor_t;

typedef enum {
    X_AXIS,
    Y_AXIS
} axis_t;

typedef enum {
    CW,
    CCW
} motor_direction_t;

typedef uint distance_t;

motor_t stepper_motor_init(axis_t axis);

motor_t stepper_motor_basic_init(axis_t axis);

void stepper_motor_smooth_step(motor_t* motor, motor_direction_t direction, uint step_time_ms, uint n_microsteps);

void stepper_motor_step(motor_t* motor, motor_direction_t direction);

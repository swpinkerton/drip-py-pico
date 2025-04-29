#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "pico/multicore.h"

// X Motor
#define X_DIR_PIN       0
#define X_STEP_PIN      1
#define X_ENABLE_PIN    16
#define X_ENDSTOP_PIN   17

// Y Motor
#define Y_DIR_PIN       6
#define Y_STEP_PIN      7
#define Y_ENABLE_PIN    20
#define Y_ENDSTOP_PIN   21

// Dropper Motor
#define DROP_DIR_PIN        19
#define DROP_STEP_PIN       19
#define DROP_ENABLE_PIN     19
#define DROP_ENDSTOP_PIN    19

// Electrodes Motor
#define ELEC_DIR_PIN        19
#define ELEC_STEP_PIN       19
#define ELEC_ENABLE_PIN     19
#define ELEC_ENDSTOP_PIN    19

// Physical Parameters
#define THREAD_PITCH_MM             0.4
#define MOTOR_STEPS_PER_REVOLUTION  24
#define MIN_STEP_TIME_MS            2
#define STEP_TIME_MS                3

// PWM Settings
#define PWM_WRAP        1024
#define N_MICROSTEPS    20

#define DEBUG

typedef struct {
    // Pins
    uint pin_dir;
    uint pin_step;
    uint pin_enable;

    // Motion
    uint location;
    uint target;
    int8_t direction;
    uint64_t next_step_time;
    uint step_time;
    bool enabled;

    // Sync
    mutex_t lock;
} motor_t;

typedef enum {
    X_AXIS,
    Y_AXIS,
    Z_ELECTROSTIM,
    Z_DROPPER
} axis_t;

typedef enum {
    MOVE,
    ZERO
} motor_command_t;

void stepper_motor_init(motor_t* motor,axis_t axis);

void motor_control_loop();

void disable_motor(motor_t* motor);

void enable_motor(motor_t* motor);

void endstop_irq_handler(uint gpio);

void move_xy(uint x, uint y);

#ifdef DEBUG
void dprintf(const char *format, ...);
#else
void dprintf(...);
#endif
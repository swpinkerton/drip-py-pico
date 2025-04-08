#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "queue.h"

// X Motor
#define X_DIR_PIN       1
#define X_STEP_PIN      2
#define X_ENABLE_PIN    3
#define X_ENDSTOP_PIN   17

// Y Motor
#define Y_DIR_PIN       4
#define Y_STEP_PIN      5
#define Y_ENABLE_PIN    6
#define Y_ENDSTOP_PIN   16

// Dropper Motor
#define DROP_DIR_PIN        0
#define DROP_STEP_PIN       0
#define DROP_ENABLE_PIN     0
#define DROP_ENDSTOP_PIN    0

// Electrodes Motor
#define ELEC_DIR_PIN        0
#define ELEC_STEP_PIN       0
#define ELEC_ENABLE_PIN     0
#define ELEC_ENDSTOP_PIN    0

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
    uint delta_steps;
    int8_t direction;
    bool enabled;
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

typedef struct {
    motor_command_t command;
    axis_t axis;
    float target;
} motor_command_packet_t;

typedef enum {
    COMPLETE,
    ERROR
} motor_response_t;

void stepper_motor_init(motor_t* motor,axis_t axis);

void stepper_motor_step(motor_t* motor, int8_t direction);

void motor_control_loop(QueueHandle_t command_queue, QueueHandle_t response_queue);

void disable_motor(motor_t* motor);

void enable_motor(motor_t* motor);

void endstop_irq_handler(uint gpio);

#ifdef DEBUG
void dprintf(const char *format, ...);
#else
void dprintf(...);
#endif
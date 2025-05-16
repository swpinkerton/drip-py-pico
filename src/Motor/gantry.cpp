#include "gantry.h"
#include "stepper.h"
#include "motor_settings.h"
#include "controller.h"
#include <stdio.h>
#include "debug.h"
#include "hardware/gpio.h"
#include "dropper.h"
#include "pico/time.h"

static motor_t x_motor;
static motor_t y_motor;

static bool gantry_resetting = false;
static bool x_reset = false;
static bool y_reset = false;

static DropperType dropper_mode = DropperType::HOSE;

static absolute_time_t x_motor_timer = {0};
static absolute_time_t y_motor_timer = {0};
static const int64_t MOTOR_ISR_DEBOUNCE_US = 6000000; // 4 seconds in microseconds

static bool is_timer_expired(absolute_time_t last_time) {
    return absolute_time_diff_us(last_time, get_absolute_time()) > MOTOR_ISR_DEBOUNCE_US;
}

int mm_to_steps(float mm) {
    DTRACE();
    float x_revolutions = mm / GANTRY_THREAD_PITCH_MM;
    return static_cast<int>(x_revolutions * GANTRY_STEPS_PER_REVOLUTION);
}

void gantry_isr(uint gpio, uint32_t event) {
    DPRINTF_TYPE(DEBUG_ISR, "Entering ISR\n");

    motor_t* motor;

    switch (gpio)
    {
    case X_ENDSTOP_PIN:
        DPRINTF_TYPE(DEBUG_ISR, "X Motor ISR\n");
        if (!is_timer_expired(x_motor_timer)) {
            DPRINTF_TYPE(DEBUG_ISR, "ISR ignored for debouncing.\n");
            return;
        }
        x_motor_timer = get_absolute_time();
        motor = &x_motor;
        x_reset = true;
        break;
    
    case Y_ENDSTOP_PIN:
        DPRINTF_TYPE(DEBUG_ISR, "Y Motor ISR\n");
        if (!is_timer_expired(y_motor_timer)) {
            DPRINTF_TYPE(DEBUG_ISR, "ISR ignored for debouncing.\n");
            return;
        }
        y_motor_timer = get_absolute_time();
        motor = &y_motor;
        y_reset = true;
        break;

    default:
        return;
        break;
    }

    DPRINTF_TYPE(DEBUG_ISR, "ISR running.\n");

    if (x_reset and y_reset) {
        x_reset = false;
        y_reset = false;
        gantry_resetting = false;
    }

    LOCK_MOTOR(motor);
    motor->location = 0;
    UNLOCK_MOTOR(motor);
    disable_motor(motor);

    // Move the gantry to it is off the endstop switch.
    set_motor_target(motor, mm_to_steps(2));
}

void init_gantry() {
    DTRACE();
    // X Motor
    motor_pins_t pins;
    pins.step = X_STEP_PIN;
    pins.enable = X_ENABLE_PIN;
    pins.dir = X_DIR_PIN;
    stepper_motor_init(&x_motor, pins);
    set_motor_target_rpm(GANTRY_STEPS_PER_REVOLUTION, GANTRY_MAX_RPM, &x_motor);
    set_motor_acceleration(GANTRY_STEPS_PER_REVOLUTION, GANTRY_ACCELERATION_RPM, &x_motor);
    add_motor(&x_motor);

    // Y Motor
    pins.step = Y_STEP_PIN;
    pins.enable = Y_ENABLE_PIN;
    pins.dir = Y_DIR_PIN;
    stepper_motor_init(&y_motor, pins);
    set_motor_target_rpm(GANTRY_STEPS_PER_REVOLUTION, GANTRY_MAX_RPM, &y_motor);
    set_motor_acceleration(GANTRY_STEPS_PER_REVOLUTION, GANTRY_ACCELERATION_RPM, &y_motor);
    add_motor(&y_motor);

    // Create the interrupts for handling the endstops.
    gpio_set_irq_enabled_with_callback(X_ENDSTOP_PIN, GPIO_IRQ_EDGE_FALL, true, gantry_isr);
    gpio_set_irq_enabled(Y_ENDSTOP_PIN, GPIO_IRQ_EDGE_FALL, true);
    gpio_pull_up(X_ENDSTOP_PIN);
    gpio_pull_up(Y_ENDSTOP_PIN);

    // Setup timers
    x_motor_timer = get_absolute_time();
    y_motor_timer = get_absolute_time();
}

void move_xy(int x, int y) {
    DTRACE();
    set_motor_target_rpm(GANTRY_STEPS_PER_REVOLUTION, GANTRY_MAX_RPM, &x_motor);
    set_motor_target_rpm(GANTRY_STEPS_PER_REVOLUTION, GANTRY_MAX_RPM, &y_motor);
    x = mm_to_steps(x);
    y = mm_to_steps(y);
    set_motor_target(&x_motor, x);
    set_motor_target(&y_motor, y);
}

void move_xy_relative(int x, int y) {
    DTRACE();
    set_motor_target_rpm(GANTRY_STEPS_PER_REVOLUTION, GANTRY_MAX_RPM, &x_motor);
    set_motor_target_rpm(GANTRY_STEPS_PER_REVOLUTION, GANTRY_MAX_RPM, &y_motor);
    x = mm_to_steps(x);
    y = mm_to_steps(y);
    set_motor_relative_target(&x_motor, x);
    set_motor_relative_target(&y_motor, y);
    
}

GantryStatus get_gantry_status() {
    // DTRACE();
    if (x_motor.enabled or y_motor.enabled) {
        return GantryStatus::MOVING;
    } else {
        return GantryStatus::STOPPED;
    }
}

void goto_well(uint x, uint y) {
    DTRACE();
    // Clamp x and y to max number of wells
    if (x > 7) {
        x = 7;
    }
    if (y > 2) {
        y = 2;
    }

    int x_distance = x * WELL_SPACING + WELL_HOME_X;
    int y_distance = y * WELL_SPACING + WELL_HOME_Y;

    switch (dropper_mode)
    {
    case DropperType::HOSE:
        x_distance += HOSE_OFFSET_X;
        y_distance += HOSE_OFFSET_Y;
        break;

    case DropperType::ELECTRODES:
        x_distance += ELECTRODES_OFFSET_X;
        y_distance += ELECTRODES_OFFSET_Y;
        break;
    
    default:
        break;
    }

    move_xy(x_distance, y_distance);
}

void set_gantry_mode(DropperType mode) {
    DTRACE();
    
    if (dropper_mode == mode) {
        // Already in the right mode, do nothing.
        return;
    }
    
    dropper_mode = mode;

    switch (dropper_mode)
    {
    case DropperType::HOSE:
        move_xy_relative(HOSE_OFFSET_X - ELECTRODES_OFFSET_X, HOSE_OFFSET_Y - ELECTRODES_OFFSET_Y);
        break;
    
    case DropperType::ELECTRODES:
        move_xy_relative(ELECTRODES_OFFSET_X - HOSE_OFFSET_X, ELECTRODES_OFFSET_Y - HOSE_OFFSET_Y);
        break;
    }
}

void reset_gantry() {
    DTRACE();
    set_motor_target_rpm(GANTRY_STEPS_PER_REVOLUTION, GANTRY_ZEROING_RPM, &x_motor);
    set_motor_target_rpm(GANTRY_STEPS_PER_REVOLUTION, GANTRY_ZEROING_RPM, &y_motor);
    gantry_resetting = true;
    set_motor_target(&x_motor, -999999);
    set_motor_target(&y_motor, -999999);
}

void wait_on_gantry_reset() {
    while(gantry_resetting) {
        tight_loop_contents();
    }
}
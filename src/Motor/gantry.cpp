#include "gantry.h"
#include "stepper.h"
#include "motor_settings.h"
#include "controller.h"
#include <stdio.h>
#include "debug.h"
#include "hardware/gpio.h"

static motor_t x_motor;
static motor_t y_motor;

static bool gantry_resetting = false;
static bool x_reset = false;
static bool y_reset = false;

void gantry_isr(uint gpio, uint32_t event) {
    DPRINTF_TYPE(DEBUG_ISR, "Entering ISR\n");

    motor_t* motor;

    switch (gpio)
    {
    case X_ENDSTOP_PIN:
        DPRINTF_TYPE(DEBUG_ISR, "X Motor ISR\n");
        motor = &x_motor;
        x_reset = true;
        break;
    
    case Y_ENDSTOP_PIN:
        DPRINTF_TYPE(DEBUG_ISR, "Y Motor ISR\n");
        motor = &y_motor;
        y_reset = true;
        break;

    default:
        return;
        break;
    }

    if (x_reset and y_reset) {
        x_reset = false;
        y_reset = false;
        gantry_resetting = false;
    }

    LOCK_MOTOR(motor);
    motor->location = 0;
    motor->target = 0;
    UNLOCK_MOTOR(motor);
    disable_motor(motor);
}

void init_gantry() {
    DTRACE();
    // X Motor
    motor_pins_t pins;
    pins.step = X_STEP_PIN;
    pins.enable = X_ENABLE_PIN;
    pins.dir = X_DIR_PIN;
    stepper_motor_init(&x_motor, pins);
    add_motor(&x_motor);

    // Y Motor
    pins.step = Y_STEP_PIN;
    pins.enable = Y_ENABLE_PIN;
    pins.dir = Y_DIR_PIN;
    stepper_motor_init(&y_motor, pins);
    add_motor(&y_motor);

    // Create the interrupts for handling the endstops.
    gpio_set_irq_enabled_with_callback(X_ENDSTOP_PIN, GPIO_IRQ_EDGE_FALL, true, gantry_isr);
    gpio_set_irq_enabled(Y_ENDSTOP_PIN, GPIO_IRQ_EDGE_FALL, true);
    gpio_pull_up(X_ENDSTOP_PIN);
    gpio_pull_up(Y_ENDSTOP_PIN);
    // gpio_set_irq_callback(gantry_isr);
}

void move_xy(int x, int y) {
    DTRACE();
    set_motor_target(&x_motor, x);
    set_motor_target(&y_motor, y);
}

GantryStatus get_gantry_status() {
    DTRACE();
    if (x_motor.enabled or y_motor.enabled) {
        return GantryStatus::MOVING;
    } else {
        return GantryStatus::STOPPED;
    }
}

void goto_well(uint x, uint y) {
    // Clamp x and y to max number of wells
    if (x > 8) {
        x = 8;
    }
    if (y > 3) {
        y = 3;
    }

    int x_distance = x * WELL_SPACING + WELL_HOME_X;
    int y_distance = y * WELL_SPACING + WELL_HOME_Y;

    move_xy(x_distance, y_distance);
}

void reset_gantry() {
    DTRACE();
    gantry_resetting = true;
    set_motor_target(&x_motor, -9999);
    set_motor_target(&y_motor, -9999);
}

void wait_on_gantry_reset() {
    while(gantry_resetting) {
        tight_loop_contents();
    }
}
#include "gantry.h"
#include "stepper.h"
#include "motor_settings.h"
#include "controller.h"

static motor_t x_motor;
static motor_t y_motor;

void gantry_isr(uint gpio, uint32_t event) {
    motor_t* motor;

    switch (gpio)
    {
    case X_ENDSTOP_PIN:
        motor = &x_motor;
        break;
    
    case Y_ENDSTOP_PIN:
        motor = &y_motor;
        break;

    default:
        return;
        break;
    }
    LOCK_MOTOR(motor)
    motor->location = 0;
    motor->target = 0;
    UNLOCK_MOTOR(motor)
    disable_motor(motor);
}

void init_gantry() {
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
    gpio_set_irq_enabled(X_ENDSTOP_PIN, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(Y_ENDSTOP_PIN, GPIO_IRQ_EDGE_FALL, true);
    gpio_pull_up(X_ENDSTOP_PIN);
    gpio_pull_up(Y_ENDSTOP_PIN);
    gpio_set_irq_callback(gantry_isr);
}

void move_xy(int x, int y) {
    set_motor_target(&x_motor, x);
    set_motor_target(&y_motor, y);
}

GantryStatus get_gantry_status() {
    if (x_motor.enabled or y_motor.enabled) {
        return GantryStatus::MOVING;
    } else {
        return GantryStatus::STOPPED;
    }
}

void reset_gantry() {
    set_motor_target(&x_motor, -9999);
    set_motor_target(&y_motor, -9999);
}
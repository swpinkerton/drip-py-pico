#include "dropper.h"
#include "controller.h"
#include "stepper.h"
#include "motor_settings.h"
#include "debug.h"
#include <stdio.h>

static motor_t z_hose_motor;
static motor_t z_electrode_motor;

void init_dropper() {
    DTRACE();
    motor_pins_t pins;
    pins.step = Z_HOSE_STEP_PIN;
    pins.enable = Z_HOSE_ENABLE_PIN;
    pins.dir = Z_HOSE_DIR_PIN;
    stepper_motor_init(&z_hose_motor, pins);
    add_motor(&z_hose_motor);

    pins.step = Z_ELECTRODE_STEP_PIN;
    pins.enable = Z_ELECTRODE_ENABLE_PIN;
    pins.dir = Z_ELECTRODE_DIR_PIN;
    stepper_motor_init(&z_electrode_motor, pins);
    add_motor(&z_electrode_motor);
}

void drop_hose() {
    DTRACE();
    set_motor_target(&z_hose_motor, 100);
}

void drop_electrodes() {
    DTRACE();
    set_motor_target(&z_electrode_motor, 100);
}

void raise_hose() {
    DTRACE();
    set_motor_target(&z_hose_motor, -100);
}

void raise_electrodes() {
    DTRACE();
    set_motor_target(&z_electrode_motor, -100);
}

DropperState get_dropper_status(DropperType type) {
    DTRACE();
    motor_t* motor_in_question;

    if (type == DropperType::HOSE) {
        motor_in_question = &z_hose_motor;
    } else {
        motor_in_question = &z_electrode_motor;
    }

    if (motor_in_question->location == -100) {
        return DropperState::UP;
    } else if (motor_in_question->location == 100) {
        return DropperState::DOWN;
    } else {
        return DropperState::MOVING;
    }
}
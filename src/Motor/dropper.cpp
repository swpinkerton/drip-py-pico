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
    set_motor_target_rpm(Z_STEPS_PER_REVOLUTION, Z_MAX_RPM, &z_hose_motor);
    set_motor_acceleration(Z_STEPS_PER_REVOLUTION, Z_ACCELERATION_RPM, &z_hose_motor);
    z_hose_motor.location = -500;
    add_motor(&z_hose_motor);

    pins.step = Z_ELECTRODE_STEP_PIN;
    pins.enable = Z_ELECTRODE_ENABLE_PIN;
    pins.dir = Z_ELECTRODE_DIR_PIN;
    stepper_motor_init(&z_electrode_motor, pins);
    set_motor_target_rpm(Z_STEPS_PER_REVOLUTION, Z_MAX_RPM, &z_electrode_motor);
    set_motor_acceleration(Z_STEPS_PER_REVOLUTION, Z_ACCELERATION_RPM, &z_electrode_motor);
    z_electrode_motor.location = -500;
    add_motor(&z_electrode_motor);
}

int height_to_steps(int height) {
    return 0;
}

void drop_hose() {
    DTRACE();
    set_motor_target(&z_hose_motor, 500);
}

void drop_electrodes() {
    DTRACE();
    set_motor_target(&z_electrode_motor, 500);
}

void raise_hose() {
    DTRACE();
    set_motor_target(&z_hose_motor, -500);
}

void raise_electrodes() {
    DTRACE();
    set_motor_target(&z_electrode_motor, -500);
}

DropperState get_dropper_status(DropperType type) {
    // DTRACE();
    motor_t* motor_in_question;

    if (type == DropperType::HOSE) {
        motor_in_question = &z_hose_motor;
    } else {
        motor_in_question = &z_electrode_motor;
    }

    if (motor_in_question->enabled) {
        return DropperState::MOVING;
    } else if (motor_in_question->location == -500) {
        return DropperState::UP;
    } else if (motor_in_question->location == 500) {
        return DropperState::DOWN;
    } else {
        return DropperState::MOVING;
    }
}
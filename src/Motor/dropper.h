#pragma once
#include "stepper.h"

enum class DropperState {
    MOVING,
    UP,
    DOWN
};

enum class DropperType {
    HOSE,
    ELECTRODES
};

struct dropper_t {
    motor_t motor;
    DropperState state;
};

DropperState get_dropper_status(DropperType type);

void init_dropper();

void raise_electrodes();
void drop_electrodes();

void raise_hose();
void drop_hose();
#include "stepper.h"

enum class GantryStatus{
    MOVING,
    STOPPED
};

void init_gantry();

void move_xy(int x, int y);

GantryStatus get_gantry_status();

void reset_gantry();
#include "stepper.h"

/**
 * @brief Gantry status
 * 
 */
enum class GantryStatus{
    MOVING,
    STOPPED
};

/**
 * @brief Initialise the gantry motors
 * 
 */
void init_gantry();

/**
 * @brief Move gantry to an (x,y) coordinate
 * 
 * @param x x distance in mm
 * @param y y distance in mm
 */
void move_xy(int x, int y);

/**
 * @brief Get the status of the Gantry
 * 
 * @return GantryStatus 
 */
GantryStatus get_gantry_status();

/**
 * @brief Reset the gantry by zero-ing the axes
 * 
 */
void reset_gantry();
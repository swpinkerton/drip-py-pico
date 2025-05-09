#pragma once
#include "stepper.h"

/** 
 * @brief Motor controller loop that runs on second core
 * 
 * This is the loop that actually steps the motors. For each motor the controller loop is controlling,
 * it will first set all direction pins, then set the step pins then finally unset all the step pins.
*/
void motor_control_loop();

/**
 * @brief Add motor to motor control list
 * 
 * @param motor Motor to be added to control list.
 */
void add_motor(motor_t* motor);

/**
 * @brief Immediately stop every motor
 * 
 * This will disable all the motor drivers and set the motor targets to be where the motors currently are.
 * 
 */
void stop_motors();

void manual_step(int num_steps);
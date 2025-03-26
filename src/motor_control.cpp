#include "motor_control.hpp"

// static queue
// current x and y position

void MotorControlTask(void *p) {
    // type cast to get the grid and the liquid state from input
    //
    while (1){
        // if liquid_state::init
        //  create queue of liquid
        //      set liquid_state::running
        // if liquid_state::running
        //      mutex the liquid_state
        //      tick off that box
        //      dispatch motor tasks
        //      mutex the liquid_state
        // if liquid state::clean
        //      clean the queue
        //      send motor to recalibrate
        //      set liquid_state::stop
        // if liquid_state::stop
        //      do nothing
    }
}

// queue creation
//  - all liquid 1, from first row, left to right
//  - all liquid 2, from first row, left to right
//  - all removals from first row, left to right

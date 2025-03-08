#ifndef INPUT_INTERFACE_HPP
#define INPUT_INTERFACE_HPP

#include "grid.hpp"
#include "input_state.cpp"


struct StateGrid {
    InputState* input_state;
    Grid* grid;
};

void PrintWelcome();

void output_grid(InputState* state, Grid* grid);

void process_liquid(InputState* state, Grid* grid, char c);

void processKeyPress(void* p);



#endif

#ifndef INPUT_INTERFACE_HPP
#define INPUT_INTERFACE_HPP

#include "grid.hpp"
#include "state.cpp"


struct StateGrid {
    State* state;
    Grid* grid;
};

void PrintWelcome();

void output_grid(State* state, Grid* grid);

void process_liquid(State* state, Grid* grid, char c);

void processKeyPress(void* p);



#endif

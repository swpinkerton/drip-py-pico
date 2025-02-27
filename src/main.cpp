#include <cstdio>
#include <hardware/adc.h>
#include <pico/stdlib.h>
#include <pico/time.h>

#include "cell.hpp"
#include "grid.hpp"
#include "state.cpp"

void output_grid(State* state, Grid* grid){
    printf("\033c");
    printf(grid->ToString().c_str());

    // if (*state == State::root || *state == State::recieving_liquid){
    printf("\x1B[37m0. Nothing\033[0m\t"); //none
    printf("\x1B[31m1. Liquid1\033[0m\t"); //liquid1
    printf("\x1B[32m2. Liquid2\033[0m\t"); //liquid2
    printf("\x1B[33m3. Extract\033[0m\t"); //extract
    printf("\n");
    // }

    if (*state == State::root){
        printf("Select a cell, column, row, or all(*)\n");
    }

    if (*state == State::recieving_liquid){
        printf("Select a liquid/process\n");
    }

    if (*state == State::recieving_quantity){
        printf("Enter a quantity(from 0-9)\n");
    }

}


void processKeyPress(State* state, Grid* grid) {
    int c = getchar_timeout_us(0); // Non-blocking read

    if (c == PICO_ERROR_TIMEOUT) {
        return; // No input available
    }

    if (*state == State::root) {
        printf("root\n");
        if (c == '\e') { // Escape sequence starts with '\e'
            if (getchar_timeout_us(0) == '[') { // Second part of sequence
                printf("arrow\n");
                switch (getchar_timeout_us(0)) { // Read the final character
                    case 'A':
                        grid->MoveCursor(0, -1);
                        break;
                    case 'B':
                        grid->MoveCursor(0, 1);
                        break;
                    case 'C':
                        grid->MoveCursor(1, 0);
                        break;
                    case 'D':
                        grid->MoveCursor(-1, 0);
                        break;
                }
            }
        } else {
            printf("else\n");

            if (c == '\n' || c == '\r') {
                printf("enter\n");
                *state = State::recieving_liquid;
            }
        }
    }
    else if (*state == State::recieving_quantity) {
        if (c >= '0' && c <= '9') {
            grid->ChangeQuantity(c - '0');
            *state = State::root;
        }
        else if (c == '\n' || c == '\r') {
            *state = State::root;
        }
    }
    else if (*state == State::recieving_liquid) {
        if (c == '0') {
            grid->ChangeLiquid(Liquid::none);
        } else if (c == '1') {
            grid->ChangeLiquid(Liquid::liquid1);
        } else if (c == '2') {
            grid->ChangeLiquid(Liquid::liquid2);
        } else if (c == '3') {
            grid->ChangeLiquid(Liquid::extract);
        }

        if ((c >= '0' && c <= '3') || (c == '\n' || c == '\r')) {
            grid->ChangeQuantity(c - '0');
            *state = State::recieving_quantity;
        }
    }
    output_grid(state, grid);
}


int main() {
    stdio_init_all();
    adc_init();
    adc_gpio_init(26);
    adc_select_input(0);
    // Cell cell = Cell(Liquid::liquid1, 0.0);
    Grid grid = Grid(3, 8);
    grid.ChangeLiquid(Liquid::liquid1);

    State state = State::root;

    output_grid(&state, &grid);
    while (1) {
        processKeyPress(&state, &grid);
        sleep_ms(10);
    }
}

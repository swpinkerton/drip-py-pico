#include "input_interface.hpp"
#include <pico/stdlib.h>

// TODO: add the ability to start the processes and stop the process
//          (stop must be come to completion on an individual well)

// TODO: refactor processKeyPress to be input_interface
// TODO: add electrostim
// TODO: refactor processKeyPress move the liquid stuff into a seperate file and
//          split off betweeen electrostim and liquid - via user pressing tab

void PrintWelcome() {
    printf("\033c");
    printf("__        __             _     _   \n");
    printf("\\ \\      / /__  ___  ___| |__ | |_ \n");
    printf(" \\ \\ /\\ / / _ \\/ _ \\/ __| '_ \\| __|\n");
    printf("  \\ V  V /  __/  __/\\__ \\ | | | |_ \n");
    printf("   \\_/\\_/ \\___|\\___||___/_| |_|\\__|\n");
    printf("                            A       \n");
    printf("                           A        \n");
    printf("      AABBBUAA            AA        \n");
    printf("     ABBBBBBBBb           ABA       \n");
    printf("     BBBBBBBABA           9BBA      \n");
    printf("     ABBBBA ABB            ABBBA    \n");
    printf("            ABB             BBBBA   \n");
    printf("            ABA             ABBBB   \n");
    printf("           ABBA            ABBBBBA  \n");
    printf("          ABBBA           dwBBBBA9  \n");
    printf("         ABBBBA    AAAAS   9f99     \n");
    printf("        ABBBBB   ABBBBBBBA          \n");
    printf("      7ABBBBBA JBBBBBBBBBAw9        \n");
    printf("     ABBBBBBA  ABBBBBBBBU99         \n");
    printf("    ABBBBBBBO  98wFLw3999           \n");
    printf("   ABBBBBBBA                        \n");
    printf("  ABBBBBBBBA                        \n");
    printf(" 9ABBBBBBBBA9                       \n");
    printf(" 07BBBBBBBBv3990                    \n");
    printf("  999el3309                         \n");
    printf("                                    \n");
    printf("Press Space Or Enter To Continue\n");
}

void output_grid(InputState* input_state, Grid* grid){
    printf("\033c");
    printf(grid->ToString().c_str());

    printf("\x1B[37m0. Nothing\033[0m\t"); //none
    printf("\x1B[31m1. Liquid1\033[0m\t"); //liquid1
    printf("\x1B[32m2. Liquid2\033[0m\t"); //liquid2
    printf("\x1B[33m3. Extract\033[0m\t"); //extract
    printf("\n");

    if (*input_state == InputState::root){
        printf("Select a cell, column, row, or all(*)\n");
    }

    if (*input_state == InputState::recieving_liquid){
        printf("Select a liquid/process\n");
    }

    if (*input_state == InputState::recieving_quantity){
        printf("Enter a quantity(from 0-9)\n");
    }

}

void process_liquid(InputState* input_state, Grid* grid, char c) {
    Liquid new_liquid = Liquid(c - '0');
    if (grid->All()){
        grid->ChangeAllLiquid(new_liquid);
    }
    if (grid->Row() != -1){
        grid->ChangeRowLiquid(new_liquid);
    }
    if (grid->Column() != -1){
        grid->ChangeColumnLiquid(new_liquid);
    }
    if (grid->All() == false && grid->Row() == -1 && grid->Column() == -1){
        grid->ChangeLiquid(new_liquid);
    }
    *input_state = InputState::recieving_quantity;
}


void processKeyPress(void* p) {
    while (1){
        struct StateGrid *sg = (struct StateGrid *) p;
        InputState* input_state = sg->input_state;
        Grid* grid = sg->grid;

        int c = getchar(); // Non-blocking read

        if (*input_state == InputState::root) {
            if (c == '\e') { // Escape sequence starts with '\e'
                if (getchar_timeout_us(0) == '[') { // Second part of sequence
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
            }
            else if (c == '\n' || c == '\r') {
                *input_state = InputState::recieving_liquid;
            }
            else if (c >= '0' && c <= '3') {
                process_liquid(input_state, grid, c);
            }
        }
        else if (*input_state == InputState::recieving_liquid) {
            if (c >= '0' && c <= '3') {
                process_liquid(input_state, grid, c);
            }
            else if (c == '\n' || c == '\r'){
                *input_state = InputState::recieving_quantity;
            }
        }
        else if (*input_state == InputState::recieving_quantity) {
            if (c >= '0' && c <= '9') {
                int new_quantity = int(c - '0');
                if (grid->All()){
                    grid->ChangeAllQuantity(new_quantity);
                }
                if(grid->Row() != -1){
                    grid->ChangeRowQuantity(new_quantity);
                }
                if(grid->Column() != -1){
                    grid->ChangeColumnQuantity(new_quantity);
                }
                if (grid->All() == false && grid->Row() == -1 && grid->Column() == -1){
                    grid->ChangeQuantity(new_quantity);
                }
                *input_state = InputState::root;
            }
            else if (c == '\n' || c == '\r') {
                *input_state = InputState::root;
            }
        }
        output_grid(input_state, grid);
    }
}

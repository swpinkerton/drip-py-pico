#include <stdio.h>

#include "hardware/gpio.h"
#include <pico/stdlib.h>
#include <pico/time.h>

#include "cell.hpp"
#include "grid.hpp"
#include "state.cpp"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#define LED_PIN 25
#define RED_LED 14

#define GPIO_ON     1
#define GPIO_OFF    0

void GreenLEDTask(void *)
{
    while (1){
        gpio_put(LED_PIN, GPIO_ON);
        vTaskDelay(1000);
        gpio_put(LED_PIN, GPIO_OFF);
        vTaskDelay(1000);
    }
}

void RedLEDTask(void *)
{
    while (1){
        gpio_put(RED_LED, GPIO_ON);
        vTaskDelay(1000);
        gpio_put(RED_LED, GPIO_OFF);
        vTaskDelay(1000);
    }
}

struct StateGrid {
    State* state;
    Grid* grid;
};

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
    // fflush(stdout); // Flush the output buffer to ensure the message is displayed immediately
}

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

void process_liquid(State* state, Grid* grid, char c) {
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
    *state = State::recieving_quantity;
}


void processKeyPress(void* p) {
    while (1){
        struct StateGrid *sg = (struct StateGrid *) p;
        State* state = sg->state;
        Grid* grid = sg->grid;

        int c = getchar(); // Non-blocking read

        if (*state == State::root) {
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
                *state = State::recieving_liquid;
            }
            else if (c >= '0' && c <= '3') {
                process_liquid(state, grid, c);
            }
        }
        else if (*state == State::recieving_liquid) {
            if (c >= '0' && c <= '3') {
                process_liquid(state, grid, c);
            }
            else if (c == '\n' || c == '\r'){
                *state = State::recieving_quantity;
            }
        }
        else if (*state == State::recieving_quantity) {
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
                *state = State::root;
            }
            else if (c == '\n' || c == '\r') {
                *state = State::root;
            }
        }
        output_grid(state, grid);
    }
}


int main() {
    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    gpio_init(RED_LED);
    gpio_set_dir(RED_LED, GPIO_OUT);

    TaskHandle_t gLEDtask = NULL;
    TaskHandle_t rLEDtask = NULL;


    // Cell cell = Cell(Liquid::liquid1, 0.0);
    Grid grid = Grid(3, 8);
    State state = State::root;

    char c = '0';
    while (c != ' ' && c != '\n' && c != '\r'){
        c = getchar();
        sleep_ms(10);
        PrintWelcome();
    }
    output_grid(&state, &grid);

    TaskHandle_t serial_task = NULL;

    StateGrid sg = {&state, &grid};

    xTaskCreate(
        processKeyPress,
        "serial",
        2048,
        &sg,
        tskIDLE_PRIORITY,
        &serial_task
    );

    xTaskCreate(
        RedLEDTask,
        "Red LED",
        1024,
        NULL,
        1,
        &rLEDtask
    );

    xTaskCreate(
        GreenLEDTask,
        "Green LED",
        1024,
        NULL,
        2,
        &gLEDtask
    );

    vTaskStartScheduler();

    // while (1) {
    //     processKeyPress(&state, &grid);
    //     sleep_ms(10);
    // }
}

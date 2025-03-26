#include "commands.cpp"
#include <cstdio>
#include <vector>

#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"


// utlaise the header file for this cpp
#define DEBUG_PRINTS 1

// #define LED_PIN 25
// #define RED_LED 14

// #define GPIO_ON     1
// #define GPIO_OFF    0

// void GreenLight() {
//     gpio_put(LED_PIN, GPIO_ON);
//     vTaskDelay(1000);
//     gpio_put(LED_PIN, GPIO_OFF);
// }

// void RedLight() {
//     gpio_put(RED_LED, GPIO_ON);
//     vTaskDelay(1000);
//     gpio_put(RED_LED, GPIO_OFF);
// }

enum InputInterfaceState {
    NEUTRAL,
    MOVE,
    LIQUID
};

void ClearQueue(QueueHandle_t commandQueue) {
    if (DEBUG_PRINTS){
        printf("clearing queue \n");
    }
    xQueueReset(commandQueue);
}


void NeutralHandler(char c, InputInterfaceState *state) {
    switch (c) {
        case 'm':
            *state = InputInterfaceState::MOVE;
            break;
        case 'l':
            *state = InputInterfaceState::LIQUID;
            break;
        default:
            break;
    }
}

int RowHandler(char c) {
    if ('0' <= c && c <= '9') {
        return c - '0';
    }
    return -1;
}

int ColumnHandler(char c) {
    if ('a' <= c && c <= 'z') {
        return c - 'a';
    }
    if ('A' <= c && c <= 'Z') {
        return c - 'A';
    }
    return -1;
}

MoveCommand MoveHandler(char c, InputInterfaceState *state) {
    if (DEBUG_PRINTS){
        printf("Move Command Started\n");
    }

    int row = RowHandler(c);
    int column = ColumnHandler(c);

    while (row == -1 || column == -1) {
        c = getchar();
        int temp_row = RowHandler(c);
        if (temp_row != -1) {
            row = temp_row;
        }
        int temp_column = ColumnHandler(c);
        if (temp_column != -1) {
            column = temp_column;
        }
    }

    *state = InputInterfaceState::NEUTRAL;
    MoveCommand mvc;
    mvc.row = row;
    mvc.column = column;
    if (DEBUG_PRINTS){
        printf("Moving to %d %d \n", row, column);
    }
    return mvc;
}


int GetCharacterAndConvertToInteger() {
    char c = getchar();
    while ('0' > c || c > '9') {
        c = getchar();
    }
    return int(c - '0');
}


LiquidCommand LiquidHandler(char c, InputInterfaceState *state) {
    if (DEBUG_PRINTS){
        printf("Liquid Command Started\n");
    }

    LiquidCommand lqc;
    // Populate liquid type and quantity
    lqc.liquid = Liquid::none;
    lqc.quantity = 1;
    if ('0' <= c && c <= '3'){
        lqc.liquid = static_cast<Liquid>(int(c - '0'));
    }
    else{
        lqc.liquid = Liquid::none;
    }

    int ten_to_the_2 = GetCharacterAndConvertToInteger();
    int ten_to_the_1 = GetCharacterAndConvertToInteger();
    int ten_to_the_0 = GetCharacterAndConvertToInteger();

    lqc.quantity = 100 * ten_to_the_2 + 10 * ten_to_the_1 + 1 * ten_to_the_0;

    *state = InputInterfaceState::NEUTRAL;
    if (DEBUG_PRINTS){
        printf("Liquid %d for %d \n", lqc.liquid, lqc.quantity);
    }

    return lqc;
}

void InputInterface(void* p) {
    QueueHandle_t * commandQueue = (QueueHandle_t *) p;

    InputInterfaceState state = NEUTRAL; // Initialize state

    while (1) {
        // receive char
        char c = getchar(); // Note: This is blocking I/O

        // check for escape button
        if (c == '\r' || c == '\n' || c == ' ') {
            ClearQueue(*commandQueue);
            continue;
        }

        // Process based on state
        switch (state) {
            case InputInterfaceState::NEUTRAL:
                NeutralHandler(c, &state);
                break;

            case InputInterfaceState::MOVE: {
                MoveCommand mq = MoveHandler(c, &state);
                Command cmd;
                cmd.type = COMMAND_MOVE;
                cmd.data.move = mq;
                if (xQueueSend(*commandQueue, &cmd, portMAX_DELAY) != pdPASS) {
                        printf("large failure sending liquid command, maybe we have a full queue?");
                }
                break;
            }

            case InputInterfaceState::LIQUID: {
                LiquidCommand lq = LiquidHandler(c, &state);
                Command cmd;
                cmd.type = COMMAND_LIQUID;
                cmd.data.liquid = lq;
                if (xQueueSend(*commandQueue, &cmd, portMAX_DELAY) != pdPASS) {
                        printf("large failure sending liquid command, maybe we have a full queue?");
                }
                break;
            }
        }
    }
}

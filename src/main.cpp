#include <stdio.h>

#include "hardware/gpio.h"
#include <pico/stdlib.h>
#include <pico/time.h>

#include "cell.hpp"
#include "grid.hpp"
#include "state.cpp"
#include "input_interface.hpp"

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

int main() {
    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    gpio_init(RED_LED);
    gpio_set_dir(RED_LED, GPIO_OUT);

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
    TaskHandle_t rLEDtask = NULL;
    TaskHandle_t gLEDtask = NULL;

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
}

#include <cstddef>
#include <stdio.h>

#include "hardware/gpio.h"
#include <pico/stdlib.h>
#include <pico/time.h>

#include "cell.hpp"
#include "grid.hpp"
#include "input_state.cpp"
#include "input_interface.cpp"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#define LED_PIN 25
#define RED_LED 14

#define GPIO_ON     1
#define GPIO_OFF    0

// TODO: add a task to handle the electro stim
//      - task notification trigered via the coms
//      - mutex the electrostim information? - if we are parallel

// TODO: add a scheduleing task to handle the current position and next positions
//      - is toggled on/off via a task notification (or something similiar) from the comms
//      - mutex the grid? - if we are parallel
//      - needs to recalabrate the motors every now and then and default to recalibration when nothing is occuring
//      - dispatches the motors (shared information and possibly a task notif?)

// TODO: add a task to hand the motors
//      - either the motors triggers a task to dispatch the liquid or they are in the same task (I prefer the later)

// TODO: add the ability for the user to bin, maybe have a start up process with liquid perging
//      - does the bin need a full warning?

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

    TaskHandle_t serial_task = NULL;
    TaskHandle_t rLEDtask = NULL;
    TaskHandle_t gLEDtask = NULL;


    xTaskCreate(
        InputInterface,
        "serial",
        2048,
        NULL,
        tskIDLE_PRIORITY,
        &serial_task
    );

    // xTaskCreate(
    //     RedLEDTask,
    //     "Red LED",
    //     1024,
    //     NULL,
    //     1,
    //     &rLEDtask
    // );

    // xTaskCreate(
    //     GreenLEDTask,
    //     "Green LED",
    //     1024,
    //     NULL,
    //     2,
    //     &gLEDtask
    // );

    vTaskStartScheduler();
}

#include <cstddef>
#include <stdio.h>

#include "hardware/gpio.h"
#include <pico/stdlib.h>
#include <pico/time.h>

#include "commands.cpp"
#include "input_interface.cpp"
#include "led.hpp"
#include "command_handler.hpp"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"


// Split the command center and LED stuff into seperate files
// Command scenter should be able to handle re-alignment and binning
//
// bin on the shorter axis (maybe recalibration for that motor can occur when binning)
// maybe the other axis recalibrated upon a move to the home of the long axis?


int main() {
    stdio_init_all();

    INITLEDs();

    TaskHandle_t serial_task = NULL;

    QueueHandle_t commandQueue = xQueueCreate(10, sizeof(Command));


    xTaskCreate(
        InputInterface,
        "serial",
        2048,
        &commandQueue,
        tskIDLE_PRIORITY,
        &serial_task
    );

    xTaskCreate(
        QueuedLEDTask,
        "RedLEDQ",
        2048,
        &commandQueue,
        1,
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

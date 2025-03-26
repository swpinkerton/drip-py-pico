#include "led.hpp"
#include "commands.cpp"
#include <stdio.h>
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "queue.h"

void QueuedLEDTask(void * p){
    QueueHandle_t * commandQueue = (QueueHandle_t *) p;
    Command receivedCommand;
    while (1){
        if (xQueueReceive(*commandQueue, &receivedCommand, portMAX_DELAY) == pdTRUE) {
            printf("recieving\n");
            // Process received commands
            if (receivedCommand.type == CommandType::COMMAND_MOVE){
                GreenLED();
            }
            else if (receivedCommand.type == CommandType::COMMAND_LIQUID){
                RedLED();
            }
        }
    }
}

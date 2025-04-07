#include <stdio.h>

#include "hardware/gpio.h"
#include <pico/stdlib.h>
#include <pico/time.h>
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"
#include "stepper.hpp"

#define DEBUG

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

QueueHandle_t motor_command_q;
QueueHandle_t motor_response_q;

void motor_control_task(void *)
{
    motor_control_loop(motor_command_q, motor_response_q);
}

void main_task(void *)
{
    // printf("Hello from main1\n");
    
    motor_command_packet_t commands[] = {
        {
            MOVE,
            X_AXIS,
            10
        },
        {
            MOVE,
            Y_AXIS,
            10
        },
        {
            MOVE,
            X_AXIS,
            5
        },
        {
            MOVE,
            Y_AXIS,
            5
        },
        {
            MOVE,
            X_AXIS,
            20
        },
        {
            MOVE,
            Y_AXIS,
            20
        },
        {
            MOVE,
            X_AXIS,
            0
        },
        {
            MOVE,
            Y_AXIS,
            0
        }
    };

    BaseType_t xStatus;
    
    int i = 0;

    motor_response_t resp;

    for (;;) {

        // Send command for X and Y.

        xStatus = xQueueSend(motor_command_q, (void*) &commands[i], 0);

        if (xStatus == pdPASS) {
            printf("Command Sent\n");
            i++;
        } else {
            printf("Command Failed\n");
        }

        xStatus = xQueueSend(motor_command_q, (void*) &commands[i], 0);

        if (xStatus == pdPASS) {
            printf("Command Sent\n");
            i++;
        } else {
            printf("Command Failed\n");
        }

        while (1) {
            // Wait until we rx confirmation.
            xStatus = xQueueReceive(motor_response_q, (void*) &resp, pdMS_TO_TICKS(1000));
            
            if (xStatus == pdPASS) {
                printf("Response received\n");
                vTaskDelay(pdMS_TO_TICKS(2000));
                
                if (i >= 7) {
                    i = 0;
                }

                break;
            } else {
                printf("timed out :(\n");
            }
            
        }
    }
}

int main() {
    stdio_init_all();

    sleep_ms(5000);

    printf("Started\n");

    motor_command_q = xQueueCreate(5, sizeof(motor_command_packet_t));
    motor_response_q = xQueueCreate(5, sizeof(motor_response_t));

    if (motor_command_q == NULL or motor_response_q == NULL) {
        printf("Queue Creation Failed\n");
        return -1;
    }

    printf("Queues created\n");

    TaskHandle_t motor_control_task_h = NULL;
    TaskHandle_t main_task_h = NULL;

    xTaskCreate(
        motor_control_task,
        "Motor Control",
        1024,
        NULL,
        1,
        &motor_control_task_h
    );

    xTaskCreate(
        main_task,
        "Main",
        1024,
        NULL,
        1,
        &main_task_h
    );

    printf("Tasks Created\n");


    vTaskStartScheduler();

    for (;;);
}

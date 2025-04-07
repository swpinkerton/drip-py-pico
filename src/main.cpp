#include <stdio.h>

#include "hardware/gpio.h"
#include <pico/stdlib.h>
#include <pico/time.h>
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"
#include "stepper.hpp"

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

void master_interrupt_handler(uint gpio, uint32_t event) {
    dprintf("master irq\n");
    switch (gpio)
    {
    case X_ENDSTOP:
        x_endstop_irq();
        break;

    case Y_ENDSTOP:
        y_endstop_irq();
        break;
    
    default:
        break;
    }
}

QueueHandle_t motor_command_q;
QueueHandle_t motor_response_q;

void motor_control_task(void *)
{
    motor_control_loop(motor_command_q, motor_response_q);
}

void main_task(void *)
{    
    
    BaseType_t xStatus;
    
    int i = 0;

    motor_response_t resp;

    for (;;) {

        // Send command for X and Y.

        char input[20];
        int x, y;

        motor_command_packet_t command_packet;

        if (fgets(input, sizeof(input), stdin)) {

            sscanf(input, "%d, %d", &x, &y);

            // Create commands for x and y motors
            command_packet.axis = X_AXIS;
            command_packet.target = x;

            xStatus = xQueueSend(motor_command_q, (void*)&command_packet, 0);
            if (xStatus == pdPASS) {
                printf("X Command Sent: %d\n", x);
                i++;
            } else {
                printf("X Command Failed\n");
            }

            command_packet.axis = Y_AXIS;
            command_packet.target = y;

            xStatus = xQueueSend(motor_command_q, (void*)&command_packet, 0);
            if (xStatus == pdPASS) {
                printf("Y Command Sent: %d\n", y);
                i++;
            } else {
                printf("Y Command Failed\n");
            }
        } else {
            printf("Invalid input. Please enter coordinates in the format 'x, y'.\n");
        }

        while (1) {
            // Wait until we rx confirmation.
            xStatus = xQueueReceive(motor_response_q, (void*) &resp, pdMS_TO_TICKS(1000));
            
            if (xStatus == pdPASS) {
                printf("Gantry arrived!\n");
                vTaskDelay(pdMS_TO_TICKS(2000));
                
                if (i >= 7) {
                    i = 0;
                }

                break;
            } else {
                dprintf("timed out :(\n");
            }
            
        }
    }
}

int main() {
    // Pico hardware setup =========================================================
    stdio_init_all();

    gpio_set_irq_callback(master_interrupt_handler);
    irq_set_enabled(IO_IRQ_BANK0, true);

    dprintf("Started\n");

    // FreeRTOS Setup ==============================================================
    // Queues
    motor_command_q = xQueueCreate(5, sizeof(motor_command_packet_t));
    motor_response_q = xQueueCreate(5, sizeof(motor_response_t));

    if (motor_command_q == NULL or motor_response_q == NULL) {
        dprintf("Queue Creation Failed\n");
        return -1;
    }

    dprintf("Queues created\n");

    // Tasks
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

    dprintf("Tasks Created\n");

    // Start the scheduler
    vTaskStartScheduler();

    // Doesn't make it here.
    for (;;) {
        printf("ruh roh something went wrong :(\n");
    }
}

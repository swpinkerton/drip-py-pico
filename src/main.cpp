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

int main() {
    stdio_init_all();

    TaskHandle_t motor_control_task_h = NULL;

    xTaskCreate(
        motor_control_task,
        "Motor Control",
        1024,
        NULL,
        1,
        &motor_control_task_h
    );

    vTaskStartScheduler();
}

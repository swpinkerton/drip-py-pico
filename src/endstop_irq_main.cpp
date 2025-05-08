#include <stdio.h>
#include "hardware/gpio.h"
#include <pico/stdlib.h>
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

void callback(uint gpio, uint32_t event) {
    switch (gpio)
    {
    case 17:
        printf("balls\n");
        break;
    case 16:
        printf("spaghetti\n");
    default:
        break;
    }
}

int main() {
    stdio_init_all();

    sleep_ms(5000);

    printf("Started\n");

    gpio_set_irq_enabled_with_callback(17, GPIO_IRQ_EDGE_FALL, true, callback);
    // gpio_set_irq_callback(callback);
    gpio_set_irq_enabled(16, GPIO_IRQ_EDGE_FALL, true);
    // gpio_set_irq_enabled(17, GPIO_IRQ_EDGE_FALL, true);
    // gpio_set_irq_callback(callback);

    gpio_pull_up(17);
    gpio_pull_up(16);

    for (;;) {
        printf("hello\n");
        sleep_ms(500);
    }
}
#include "debug.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "gantry.h"
#include "dropper.h"
#include "controller.h"
#include "stepper.h"
#include <vector>

// #include <queue>
// #include <string>

// using std::queue;
// using std::string;

using std::vector;

void core1_entry() {
    motor_control_loop();
}

// void run_command(motor_command_t command) {

// }

// void send_command_response(motor_command_t command) {

// }

int main() {
    stdio_init_all();

    printf("Core 0 Started...\n");

    init_gantry();
    init_dropper();

    multicore_launch_core1(core1_entry);

    // bool running_command = false;
    // motor_command_t current_command;
    // queue<motor_command_t> command_q;

    while(1) {

        move_xy(1000,1000);
        sleep_ms(10000);
        move_xy(0,0);
        sleep_ms(10000);
        // Check for new input

        // If not executing command,
        // Execute top of command queue
        // if (!running_command and !command_q.empty()) {
        //     current_command = command_q.front();
        //     run_command(current_command);
        //     command_q.pop();
        //     running_command = true;
        // }

        // // check if current command is finished.
        // // If so report move complete.
        // if (running_command and check_status() == STOPPED) {
        //     send_command_response(current_command);
        //     running_command = false;
        // }
    }   
}
#include "debug.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "stepper.hpp"
#include <queue>
#include <string>

using std::queue;
using std::string;

void core1_entry() {
    motor_control_loop();
}

void run_command(motor_command_t command) {

}

void send_command_response(motor_command_t command) {

}

int main() {
    stdio_init_all();

    printf("Core 0 Started...\n");

    multicore_launch_core1(core1_entry);

    bool running_command = false;
    motor_command_t current_command;
    queue<motor_command_t> command_q;

    while(1) {
        // Check for new input

        // If not executing command,
        // Execute top of command queue
        if (!running_command and !command_q.empty()) {
            current_command = command_q.front();
            run_command(current_command);
            command_q.pop();
            running_command = true;
        }

        // check if current command is finished.
        // If so report move complete.
        if (running_command and check_status() == STOPPED) {
            send_command_response(current_command);
            running_command = false;
        }
    }   
}
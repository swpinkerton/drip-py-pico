#include "debug.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "pico/multicore.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/uart.h"
#include "gantry.h"
#include "dropper.h"
#include "controller.h"
#include "stepper.h"
#include <vector>
#include <queue>
#include <string>
#include <sstream>

using std::queue;
using std::string;
using std::stringstream;
using std::vector;

void core1_entry() {
    motor_control_loop();
}

enum class CommandReadState {
    WAITING,
    COMMAND,
    ARGUMENTS
};

enum class Component {
    GANTRY,
    HOSE,
    ELECTODES,
    NONE,
    ERROR
};

enum class State {
    IDLE,
    RUNNING_COMMAND
};

int process_command(string command, Component& active_component) {
    stringstream ss(command);
    string cmd;
    ss >> cmd;

    if (cmd.empty()) {
        printf("Empty command received.\n");
        return 0;
    }

    if (cmd == "GOTO_WELL") 
    {
        int x, y;
        if (ss >> x >> y) {
            DSTATUS("GOTO_WELL %d %d command received\n", x, y);
            goto_well(x, y);
            active_component = Component::GANTRY;
            return 1;
        } else {
            DWARNING("Bad arguments for GOTO_WELL command\n");
        }
    } else if (cmd == "MOVE")
    {
        int x, y;
        if (ss >> x >> y) {
            DSTATUS("MOVE %d %d command received\n", x, y);
            move_xy(x, y);
            active_component = Component::GANTRY;
        } else {
            DWARNING("Bad arguments for MOVE command\n");
        }
    } else if (cmd == "DROP")
    {
        DSTATUS("DROP command received\n");

    } else if (cmd == "RAISE")
    {
        DSTATUS("RAISE command received\n");

    } else if (cmd == "ZERO")
    {
        string component;
        if (!(ss >> component)) {
            DWARNING("Bad arguments for ZERO command\n");
            return 0;
        }
        DSTATUS("ZERO %s command received\n", component.c_str());

        if (component == "GANTRY") {
            reset_gantry();
            active_component = Component::GANTRY;
            return 1;
        }
    } else if (cmd == "GET_STATUS")
    {

    } else if (cmd == "STOP") 
    {
        printf("STOP command received.\n");
        stop_motors();
    } else if (cmd == "STEP")
    {
        int x;
        if (ss >> x) {
            DSTATUS("STEP %d command received\n", x);
            manual_step(x);
            active_component = Component::GANTRY;
            return 1;
        } else {
            DWARNING("Bad arguments for STEP command\n");
        }
    } else {
        printf("Unknown command: %s\n", cmd.c_str());
    }
    return 0;
}

void send_command_complete_response(string command) {
    DSTATUS("Command \"%s\"complete\n", command.c_str());
    printf("COMMAND COMPLETE: %s\n", command.c_str());
}

int main() {
    stdio_init_all();

    printf("Core 0 Started...\n");

    init_gantry();
    init_dropper();

    multicore_launch_core1(core1_entry);

    State state = State::IDLE;
    string current_command_string;
    Component active_component = Component::NONE;

    queue<string> command_queue;
    string input_buffer;

    while(1) {
        // Check for new input
        int c = getchar_timeout_us(0);

        if (c != PICO_ERROR_TIMEOUT and c != '\r') {
            if (c == '\n') {
                // Command read finished, process it.
                if (process_command(input_buffer, active_component)) {
                    DSTATUS("Running Command\n");
                    state = State::RUNNING_COMMAND;
                    current_command_string = input_buffer;
                } else {
                    DWARNING("Process command failed\n");
                }
                input_buffer.clear();
            } else {
                input_buffer += c;
            }
        }

        if (state == State::RUNNING_COMMAND) {
            switch (active_component)
            {
            case Component::GANTRY:
                if (get_gantry_status() == GantryStatus::STOPPED) {
                    state = State::IDLE;
                    send_command_complete_response(current_command_string);
                    current_command_string = "";
                }
                break;
            
            default:
                break;
            }
        }
    }
}
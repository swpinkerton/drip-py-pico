// pico_2_pump_control.c

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"   // Required for PWM functionality
#include "string.h"         // Required for string manipulation functions
#include <ctype.h>          // Required for isprint

// UART for communication with the Aladdin pump
#define PUMP_ID uart0
#define PUMP_RATE 300 // lowered from 19200 to 300 on pump

// Pins for UART0 connected to the pump's RS-232 interface (via a level shifter if needed)
// Physical Pin 21 is GPIO 16 (TX), Physical Pin 22 is GPIO 17 (RX)
#define PUMP_TX_PIN 16
#define PUMP_RX_PIN 17
// --- End UART Configuration ---

// PWM Configuration
#define PWM_PIN 3 // GPIO pin for PWM output
#define PWM_FREQ_HZ 50.0f // Target PWM frequency in Hz (20ms period)

// Calculated PWM values for 50Hz (based on 125MHz system clock)
// With PWM_PRESCALER_INT = 100, and PWM_WRAP_VALUE = 24999:
// Actual Frequency = 125,000,000 / (100 * (24999 + 1)) = 125,000,000 / (100 * 25000) = 50 Hz
#define PWM_PRESCALER_INT 100
#define PWM_WRAP_VALUE 24999

// Pulse widths in PWM ticks (level argument for pwm_set_chan_level)
// level = (desired_pulse_ms / 20ms) * (PWM_WRAP_VALUE + 1)
#define PULSE_1MS_TICKS   (uint16_t)((0.7f / 20.0f) * (PWM_WRAP_VALUE + 1))     //left
#define PULSE_1_5MS_TICKS (uint16_t)((1.5f / 20.0f) * (PWM_WRAP_VALUE + 1))     //centre
#define PULSE_2MS_TICKS   (uint16_t)((2.49f / 20.0f) * (PWM_WRAP_VALUE + 1))    //right

// Global variables to store PWM slice and channel for easy access
uint pwm_slice_num;
uint pwm_channel;

// Function to send a command to the pump and read its response (UNCHANGED)
// Returns 0 on success (receives ETX), -1 on timeout or buffer full before ETX
int send_pump_command(const char* command, char* response_buffer, size_t buffer_size, uint send_delay_ms, uint read_timeout_ms) {
    // Clear the response buffer and ensure null termination
    memset(response_buffer, 0, buffer_size);
    response_buffer[0] = '\0'; // Explicitly null-terminate at the start

    // Clear UART receive buffer before sending to avoid reading old data
    while(uart_is_readable(PUMP_ID)) {
        uart_getc(PUMP_ID);
    }

    // Send the command to the pump
    uart_puts(PUMP_ID, command);
    printf("Sent to pump: %s", command); // Print command sent for debugging

    // Wait a moment for the pump to process and start sending a response
    sleep_ms(send_delay_ms);

    // Wait for a response, looking for ETX (0x03) as the end delimiter
    uint start_time = to_ms_since_boot(get_absolute_time());
    size_t response_len = 0;
    bool etx_received = false;

    while (to_ms_since_boot(get_absolute_time()) - start_time < read_timeout_ms) {
        if (uart_is_readable(PUMP_ID)) {
            char c = uart_getc(PUMP_ID);

            // Store the received character if buffer has space
            if (response_len < buffer_size - 1) {
                response_buffer[response_len++] = c;
            } else {
                // Buffer is full, stop reading to prevent overflow
                printf("Response buffer full before receiving ETX.\n");
                break;
            }

            // Check for ETX (End of Text) character
            if (c == 0x03) {
                etx_received = true;
                // We received ETX, but let's add a small delay to grab any potential trailing chars
                sleep_ms(50);
                // Now read any remaining characters that might have arrived in the last 50ms
                 while(uart_is_readable(PUMP_ID) && response_len < buffer_size - 1) {
                    response_buffer[response_len++] = uart_getc(PUMP_ID);
                }
                break; // Exit the loop as we received ETX (and potentially trailing chars)
            }
        }
        // Small sleep to prevent busy-waiting
        sleep_us(10);
    }

    // Null-terminate the received string
    response_buffer[response_len] = '\0';

    // Print the raw received bytes in hex and as a string for debugging
    printf("Received from pump (%zu bytes): ", response_len);
    for(size_t i = 0; i < response_len; i++) {
        printf("0x%02X ", response_buffer[i]);
    }
    printf("String: '");
    for(size_t i = 0; i < response_len; i++) {
        printf("%c", isprint(response_buffer[i]) ? response_buffer[i] : '.');
    }
     printf("'\n");


    if (etx_received) {
        printf("ETX (0x03) received. Command likely processed.\n");
        return 0; // Success, ETX received
    } else {
        printf("ETX (0x03) not received within timeout.\n");
        return -1; // Error, ETX not received
    }
}

// Function to set PWM pulse width and print status
void set_pwm_pulse_width(uint16_t pulse_ticks) {
    pwm_set_chan_level(pwm_slice_num, pwm_channel, pulse_ticks);
    // Calculate pulse width in ms for display
    float pulse_ms = (float)pulse_ticks / (PWM_WRAP_VALUE + 1) * (1000.0f / PWM_FREQ_HZ);
    printf("PWM pulse set to %hu ticks (%.2f ms) on GP%d.\n", pulse_ticks, pulse_ms, PWM_PIN);
}


int main() {
    // Initialize standard STDIO over USB (for printf and putchar)
    stdio_init_all();

    // --- UART Initialization ---
    uart_init(PUMP_ID, PUMP_RATE);
    gpio_set_function(PUMP_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(PUMP_RX_PIN, GPIO_FUNC_UART);

    // --- PWM Initialization ---
    gpio_set_function(PWM_PIN, GPIO_FUNC_PWM); // Set GP28 to PWM function
    pwm_slice_num = pwm_gpio_to_slice_num(PWM_PIN); // Get the PWM slice for GP28
    pwm_channel = pwm_gpio_to_channel(PWM_PIN);     // Get the PWM channel for GP28

    // Set PWM frequency (period = 20ms)
    pwm_set_wrap(pwm_slice_num, PWM_WRAP_VALUE);
    pwm_set_clkdiv_int_frac(pwm_slice_num, PWM_PRESCALER_INT, 0); // Integer prescaler, no fraction

    // Set default pulse width (1.5ms)
    set_pwm_pulse_width(PULSE_1_5MS_TICKS);

    // Enable the PWM
    pwm_set_enabled(pwm_slice_num, true);

    // Add a longer initial delay after initialization
    sleep_ms(5000);

    printf("Pico 2 Pump Control Ready.\n");
    printf("Connect pump to UART0 (GPIO %d TX, %d RX).\n", PUMP_TX_PIN, PUMP_RX_PIN); // Corrected message for GPIO numbers
    printf("Ensure pump is powered on and configured for address 0, %d baud.\n", PUMP_RATE);
    printf("Press 's' to send the command sequence (DIA, RAT, RUN).\n");
    printf("Press 'l' for 1ms pulse, 'c' for 1.5ms (default), 'r' for 2ms on GP%d.\n", PWM_PIN);


    char pump_response[64]; // Buffer to store pump responses
    uint send_delay = 100; // Increased delay after sending command before reading response
    uint read_timeout = 1500; // Increased timeout for reading the response

    while (1) {
        // Check for a character from USB serial (non-blocking)
        int c = getchar_timeout_us(0); // Read character with a 0 timeout (non-blocking)

        if (c == PICO_ERROR_TIMEOUT) {
            // No character received, continue loop after a small delay
            sleep_ms(10);
            continue;
        }

        // Handle pump command trigger
        if (c == 's' || c == 'S') {
            printf("\n--- Trigger received. Sending command sequence... ---\n");

            // Add a small delay before sending the first command in the sequence
            sleep_ms(200); // Increased delay

            // --- Command 1: Set Diameter ---
            printf("Attempting to set Diameter...\n");
            // Use "00" for the pump address
            if (send_pump_command("00DIA12.3\r", pump_response, sizeof(pump_response), send_delay, read_timeout) == 0) {
                // Check the received response content after ETX
                // A successful SET command in Basic mode should return address + status + ETX (e.g., "00S\x03")
                // Use the local variable pump_response here
                if (pump_response[0] == 0x02 && pump_response[1] == '0' && pump_response[2] == '0' &&
                    (pump_response[3] == 'S' || pump_response[3] == 'P' || pump_response[3] == 'I' || pump_response[3] == 'W') &&
                     pump_response[4] == 0x03) {
                     printf("DIA command successful (received expected Basic response).\n");
                     sleep_ms(250); // Small delay before next command
                } else {
                    printf("DIA command response unexpected. Aborting sequence.\n");
                    // The raw hex output will help diagnose what was received
                    continue; // Skip to next iteration, wait for another trigger
                }
            } else {
                printf("Failed to send DIA command or receive ETX within timeout. Aborting sequence.\n");
                 continue; // Skip to next iteration, wait for another trigger
            }

            // --- Command 2: Set Rate ---
             printf("\nAttempting to set Rate...\n");
             // Use "00" for the pump address
            if (send_pump_command("00RAT1.10MM\r", pump_response, sizeof(pump_response), send_delay, read_timeout) == 0) {
                 // Use the local variable pump_response here
                 if (pump_response[0] == 0x02 && pump_response[1] == '0' && pump_response[2] == '0' &&
                    (pump_response[3] == 'S' || pump_response[3] == 'P' || pump_response[3] == 'I' || pump_response[3] == 'W') &&
                     pump_response[4] == 0x03) {
                    printf("RAT command successful (received expected Basic response).\n");
                    sleep_ms(250); // Small delay before next command
                } else {
                    printf("RAT command response unexpected. Aborting sequence.\n");
                     continue; // Skip to next iteration
                }
            } else {
                 printf("Failed to send RAT command or receive ETX within timeout. Aborting sequence.\n");
                  continue; // Skip to next iteration
            }

            // --- Command 3: Run Pump ---
            printf("\nAttempting to Run Pump...\n");
            // Use "00" for the pump address
            if (send_pump_command("00RUN\r", pump_response, sizeof(pump_response), send_delay, read_timeout) == 0) {
                 // For RUN, the status should change to I (Infusing) or W (Withdrawing)
                 // Use the local variable pump_response here
                 if (pump_response[0] == 0x02 && pump_response[1] == '0' && pump_response[2] == '0' &&
                    (pump_response[3] == 'I' || pump_response[3] == 'W') &&
                     pump_response[4] == 0x03) {
                    printf("RUN command successful. Pump should be running.\n");
                    // Pump is running, you might want to send a STOP command later
                    // or implement logic to monitor pump status.
                } else {
                    printf("RUN command response unexpected. Pump may not have started.\n");
                }
            } else {
                 printf("Failed to send RUN command or receive ETX within timeout.\n");
            }

            printf("\n--- Command sequence finished. ---\n");
        }
        // Handle PWM pulse width changes based on input
        else if (c == 'l' || c == 'L') {
            set_pwm_pulse_width(PULSE_1MS_TICKS);
        } else if (c == 'c' || c == 'C') {
            set_pwm_pulse_width(PULSE_1_5MS_TICKS);
        } else if (c == 'r' || c == 'R') {
            set_pwm_pulse_width(PULSE_2MS_TICKS);
        }
    }

    return 0;
}

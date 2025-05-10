// pico_2_receiver.c

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"

// --- Configuration ---
#define UART_ID uart0
#define PUMP_ID uart0
#define BAUD_RATE 115200 // Make sure this matches Pico 1
#define PUMP_RATE 19200 // Make sure this matches Pico 1


// Use UART0 default pins
#define UART_TX_PIN 0 // Connect this to Pico 1's RX pin
#define UART_RX_PIN 1 // Connect this to Pico 1's TX pin

#define PUMP_TX_PIN 4
#define PUMP_RX_PIN 5
// --- End Configuration ---

int main() {
    // Initialize standard STDIO over USB (for printf and putchar)
    stdio_init_all();

    // Initialize the chosen UART
    uart_init(UART_ID, BAUD_RATE);
    uart_init(PUMP_ID, PUMP_RATE);


    // Set the TX and RX pins functions for the UART
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    gpio_set_function(PUMP_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(PUMP_RX_PIN, GPIO_FUNC_UART);


    // Optional: A small delay to allow USB serial connection to establish
    sleep_ms(2000); // Uncomment if needed

    printf("Pico 2 Receiver Ready. Waiting for data from Pico 1 via UART...\n");

    int counter = 0;
    while (1) {
        // Check if there is data available to read from UART
        if (uart_is_readable(UART_ID)) {
            // Read one character from the UART (blocking if called when readable)
            char ch = uart_getc(UART_ID);

            // Print the received character to the USB serial output
            putchar(ch);
            if (counter == 0){
                uart_puts(PUMP_ID, "0 DIA 12.3\r");
            }
            if (counter == 1){
                uart_puts(PUMP_ID, "0 RAT 1.10 MM\r");
            }
            if (counter == 2){
                uart_puts(PUMP_ID, "0 RUN\r");
            }
            counter = counter + 1;
            if (counter == 3){
                counter = 0;
            }


            fflush(stdout); // Ensure the character is sent out over USB immediately
        }
         // You might want a small sleep here to prevent hogging the CPU if no data arrives
         // sleep_ms(1); // Uncomment if needed
    }

    return 0; // Should never reach here
}

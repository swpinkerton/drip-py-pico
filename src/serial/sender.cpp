// pico_1_sender.c

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"

// --- Configuration ---
#define UART_ID uart0
#define BAUD_RATE 115200 // Make sure this matches Pico 2

// Use UART0 default pins
#define UART_TX_PIN 0
#define UART_RX_PIN 1
// --- End Configuration ---

int main() {
    // Initialize standard STDIO over USB (for printf and getchar)
    stdio_init_all();

    // Initialize the chosen UART
    uart_init(UART_ID, BAUD_RATE);

    // Set the TX and RX pins functions for the UART
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // Optional: A small delay to allow USB serial connection to establish
    sleep_ms(2000); // Uncomment if needed

    printf("Pico 1 Sender Ready. Type characters here to send to Pico 2 via UART.\n");

    while (1) {
        // Read a character from the USB serial input (blocking)
        int c = getchar();

        // Check if a valid character was received (getchar returns PICO_ERROR_TIMEOUT or other negatives on error/timeout)
        if (c >= 0) {
            // Send the character over the hardware UART
            // uart_putc_raw waits until space is available
            uart_putc_raw(UART_ID, (char)c);

            // Optional: Echo the character back to the USB serial for confirmation
            // putchar(c);
            // fflush(stdout); // Ensure it gets sent immediately if echoing
        }
         // You might want a small sleep here if the loop runs too fast without input
         // sleep_ms(1); // Uncomment if needed, e.g., to reduce CPU usage slightly
    }

    return 0; // Should never reach here
}

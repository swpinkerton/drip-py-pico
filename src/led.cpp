#include "led.hpp"

#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"


void INITLEDs(){
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    gpio_init(RED_LED);
    gpio_set_dir(RED_LED, GPIO_OUT);
}


void GreenLED(){
    gpio_put(LED_PIN, GPIO_ON);
    vTaskDelay(1000);
    gpio_put(LED_PIN, GPIO_OFF);
    vTaskDelay(1000);
}


void GreenLEDTask(void *)
{
    while (1){
        GreenLED();
    }
}


void RedLED(){
    gpio_put(RED_LED, GPIO_ON);
    vTaskDelay(1000);
    gpio_put(RED_LED, GPIO_OFF);
    vTaskDelay(1000);
}


void RedLEDTask(void *)
{
    while (1){
        RedLED();
    }
}

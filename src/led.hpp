#ifndef LED_HPP
#define LED_HPP

#define LED_PIN 25
#define RED_LED 14

#define GPIO_ON     1
#define GPIO_OFF    0

void INITLEDs();

void GreenLED();

void GreenLEDTask(void *);

void RedLED();

void RedLEDTask(void *);

#endif

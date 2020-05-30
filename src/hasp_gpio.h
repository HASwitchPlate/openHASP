#ifndef HASP_GPIO_H
#define HASP_GPIO_H

#include "ArduinoJson.h"

#ifdef __cplusplus
extern "C" {
#endif

void gpioSetup(void);
void IRAM_ATTR gpioLoop(void);
void gpio_set_group_outputs(uint8_t groupid, uint8_t eventid);

bool gpioGetConfig(const JsonObject & settings);
bool gpioSetConfig(const JsonObject & settings);

#define HASP_GPIO_FREE 0x00
#define HASP_GPIO_USED 0x01
#define HASP_GPIO_SWITCH 0x02
#define HASP_GPIO_SWITCH_INVERTED 0x03
#define HASP_GPIO_BUTTON 0x04
#define HASP_GPIO_INPUT_BUTTON_INVERTED 0x05
#define HASP_GPIO_COUNTER 0x06
#define HASP_GPIO_COUNTER_INVERTED 0x07
#define HASP_GPIO_ADC 0x08
#define HASP_GPIO_ADC_INVERTED 0x09
#define HASP_GPIO_RELAY 0x0A
#define HASP_GPIO_RELAY_INVERTED 0x0B
#define HASP_GPIO_LED 0x0C
#define HASP_GPIO_LED_INVERTED 0x0D
#define HASP_GPIO_PWM 0x0E
#define HASP_GPIO_PWM_INVERTED 0x0F
#define HASP_GPIO_DAC 0x10
#define HASP_GPIO_DAC_INVERTED 0x11
#define HASP_GPIO_USER 0xFF

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
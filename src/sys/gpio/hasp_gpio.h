/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_GPIO_H
#define HASP_GPIO_H

#include "ArduinoJson.h"

#ifdef __cplusplus
extern "C" {
#endif

struct hasp_gpio_config_t
{
    uint8_t pin;           // pin number
    uint8_t group;         // groupid
    uint8_t type;          // switch, button, ...
    uint8_t gpio_function; // INPUT, OUTPUT, PULLUP, etc
    uint16_t val;
    uint16_t max;
};

void gpioSetup(void);
void gpioLoop(void);
void gpioEvery5Seconds(void);

// void gpio_set_group_onoff(uint8_t groupid, bool ison);
void gpio_set_normalized_group_value(uint8_t groupid, int16_t val, int16_t min, int16_t max);
// void gpio_set_gpio_state(uint8_t pin, uint16_t state);
bool gpio_get_value(uint8_t pin, uint16_t& val);
bool gpio_set_pin_value(uint8_t pin, int32_t val);
void gpio_set_moodlight(uint8_t r, uint8_t g, uint8_t b);

void gpio_discovery(JsonArray& relay, JsonArray& led);

bool gpioSavePinConfig(uint8_t config_num, uint8_t pin, uint8_t type, uint8_t group, uint8_t pinfunc);
bool gpioIsSystemPin(uint8_t gpio);
bool gpioInUse(uint8_t gpio);
bool gpioConfigInUse(uint8_t num);
int8_t gpioGetFreeConfigId();
hasp_gpio_config_t gpioGetPinConfig(uint8_t num);

#if HASP_USE_CONFIG > 0
bool gpioGetConfig(const JsonObject& settings);
bool gpioSetConfig(const JsonObject& settings);
#endif

#define HASP_GPIO_FREE 0x00
#define HASP_GPIO_USED 0x01
#define HASP_GPIO_SWITCH 0x02 // User Inputs
#define HASP_GPIO_SWITCH_INVERTED 0x03
#define HASP_GPIO_BUTTON 0x04
#define HASP_GPIO_BUTTON_INVERTED 0x05
#define HASP_GPIO_TOUCH 0x06
#define HASP_GPIO_TOUCH_INVERTED 0x07
#define HASP_GPIO_COUNTER_RISE 0x10 // User Counters
#define HASP_GPIO_COUNTER_RISE_INVERTED 0x11
#define HASP_GPIO_COUNTER_FALL 0x12
#define HASP_GPIO_COUNTER_FALL_INVERTED 0x13
#define HASP_GPIO_COUNTER_BOTH 0x14
#define HASP_GPIO_COUNTER_BOTH_INVERTED 0x15
#define HASP_GPIO_RELAY 0x20 // User Outputs
#define HASP_GPIO_RELAY_INVERTED 0x21
#define HASP_GPIO_LED 0x22
#define HASP_GPIO_LED_INVERTED 0x23
#define HASP_GPIO_LED_R 0x24
#define HASP_GPIO_LED_R_INVERTED 0x25
#define HASP_GPIO_LED_G 0x26
#define HASP_GPIO_LED_G_INVERTED 0x27
#define HASP_GPIO_LED_B 0x28
#define HASP_GPIO_LED_B_INVERTED 0x29
#define HASP_GPIO_LED_W 0x2A
#define HASP_GPIO_LED_W_INVERTED 0x2B
#define HASP_GPIO_LED_WW 0x2C
#define HASP_GPIO_LED_WW_INVERTED 0x2D
#define HASP_GPIO_LED_CW 0x2E
#define HASP_GPIO_LED_CW_INVERTED 0x2F
#define HASP_GPIO_BUZZER 0x30
#define HASP_GPIO_BUZZER_INVERTED 0x31
#define HASP_GPIO_HAPTIC 0x32
#define HASP_GPIO_HAPTIC_INVERTED 0x33
#define HASP_GPIO_PWM 0x40
#define HASP_GPIO_PWM_INVERTED 0x41
#define HASP_GPIO_DAC 0x50
#define HASP_GPIO_DAC_INVERTED 0x51
#define HASP_GPIO_ADC 0x52
#define HASP_GPIO_ADC_INVERTED 0x53
#define HASP_GPIO_SERIAL_DIMMER 0x60
#define HASP_GPIO_USER 0xFF

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
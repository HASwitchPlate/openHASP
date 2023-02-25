/* MIT License - Copyright (c) 2019-2023 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_GPIO_H
#define HASP_GPIO_H

#include "hasplib.h"

#ifdef ARDUINO
#include "AceButton.h"
using namespace ace_button;
#endif

struct hasp_gpio_config_t
{
    uint8_t pin : 8;           // pin number
    uint8_t group : 8;         // groupid
    uint8_t gpio_function : 7; // INPUT, OUTPUT, PULLUP, etc
    uint8_t type; // switch, button, ...
    uint8_t inverted : 1;
    uint8_t channel : 4; // pwmchannel
    uint8_t power : 1;
    uint16_t val;
    uint16_t max;
#ifdef ARDUINO
    AceButton* btn;
#endif
};

extern hasp_gpio_config_t gpioConfig[HASP_NUM_GPIO_CONFIG];

#ifdef __cplusplus
extern "C" {
#endif

void gpioSetup(void);
IRAM_ATTR void gpioLoop(void);
void gpioEvery5Seconds(void);

void gpio_set_normalized_group_values(hasp_update_value_t& value);
void gpio_output_group_values(uint8_t group);

bool gpio_input_pin_state(uint8_t pin);
bool gpio_output_pin_state(uint8_t pin);
bool gpio_get_pin_state(uint8_t pin, bool& power, int32_t& val);
bool gpio_set_pin_state(uint8_t pin, bool power, int32_t val);

void gpio_set_moodlight(moodlight_t& moodlight);

void gpio_discovery(JsonObject& input, JsonArray& relay, JsonArray& light, JsonArray& dimmer);

bool gpioSavePinConfig(uint8_t config_num, uint8_t pin, uint8_t type, uint8_t group, uint8_t pinfunc, bool inverted);
bool gpioIsSystemPin(uint8_t gpio);
bool gpioInUse(uint8_t pin);
bool gpioConfigInUse(uint8_t num);
int8_t gpioGetFreeConfigId();
hasp_gpio_config_t gpioGetPinConfig(uint8_t num);

#if HASP_USE_CONFIG > 0
bool gpioGetConfig(const JsonObject& settings);
bool gpioSetConfig(const JsonObject& settings);
#endif

enum hasp_gpio_function_t {
    OUTPUT_PIN        = 1,
    INTERNAL_PULLUP   = 2,
    INTERNAL_PULLDOWN = 3,
    EXTERNAL_PULLUP   = 4,
    EXTERNAL_PULLDOWN = 5
};

enum hasp_gpio_type_t {
    FREE = 0x00,
    USED = 0x01,

    /* Outputs */
    LED                          = 0x02,
    LED_R                        = 0x03,
    LED_G                        = 0x04,
    LED_B                        = 0x05,
    LED_CW                       = 0x06,
    LED_WW                       = 0x07,
    LED_W                        = 0x08,
    LIGHT_RELAY                  = 0x0A,
    POWER_RELAY                  = 0x0B,
    SHUTTER_RELAY                = 0x0C,
    SHUTTER_OPEN                 = 0x1A,
    SHUTTER_CLOSE                = 0x1B,
    BACKLIGHT                    = 0x20,
    PWM                          = 0x21,
    HASP_DAC                     = 0x22,
    SERIAL_DIMMER                = 0x30,
    SERIAL_DIMMER_L8_HD_INVERTED = 0x31,
    SERIAL_DIMMER_L8_HD          = 0x32,
    BUZZER                       = 0x40,
    HAPTIC                       = 0x41,

    AWNING  = 0x89,
    BLIND   = 0x8A,
    CURTAIN = 0x8B,
    DAMPER  = 0x8C,
    GATE    = 0x8D,
    SHADE   = 0x8E,
    SHUTTER = 0x8F,

    /* Inputs */
    SWITCH           = 0xA0, // Binary Sensors
    BATTERY          = 0xA1,
    BATTERY_CHARGING = 0xA2,
    COLD             = 0xA3,
    CONNECTIVITY     = 0xA4,
    DOOR             = 0xA5,
    GARAGE_DOOR      = 0xA6,
    GAS              = 0xA7,
    HEAT             = 0xA8,
    LIGHT            = 0xA9,
    LOCK             = 0xAA,
    MOISTURE         = 0xAB,
    MOTION           = 0xAC,
    MOVING           = 0xAD,
    OCCUPANCY        = 0xAE,
    OPENING          = 0xAF,
    PLUG             = 0xB0,
    POWER            = 0xB1,
    PRESENCE         = 0xB2,
    PROBLEM          = 0xB3,
    SAFETY           = 0xB4,
    SMOKE            = 0xB5,
    SOUND            = 0xB6,
    VIBRATION        = 0xB7,
    WINDOW           = 0xB8,
    CARBON_MONOXIDE  = 0xB9,
    RUNNING          = 0xBA,
    TAMPER           = 0xBB,
    UPDATE           = 0xBC,

    BUTTON             = 0xF0,
    BUTTON_TOGGLE_UP   = 0xF1,
    BUTTON_TOGGLE_DOWN = 0xF2,
    BUTTON_TOGGLE_BOTH = 0xF3,
    TOUCH              = 0xF4,

    HASP_ADC = 0xF9,

    COUNTER_RISE = 0xFA, // User Counters
    COUNTER_FALL = 0xFB,
    COUNTER_BOTH = 0xFC,

    USER = 0xFF
};

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
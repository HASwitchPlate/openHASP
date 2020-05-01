#ifndef HASP_GPIO_H
#define HASP_GPIO_H

#include "ArduinoJson.h"

#ifdef __cplusplus
extern "C" {
#endif

void gpioSetup(void);
void IRAM_ATTR gpioLoop(void);

enum lv_hasp_gpio_type_t {
    HASP_GPIO_SWITCH    = 0,
    HASP_GPIO_BUTTON    = 1,
    HASP_GPIO_RELAY     = 2,
    HASP_GPIO_PWM       = 3,
    HASP_GPIO_BACKLIGHT = 4,
};

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
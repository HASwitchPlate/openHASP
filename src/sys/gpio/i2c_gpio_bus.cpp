/* MIT License - Copyright (c) 2019-2024 Francis Van Roie */

#include "hasp_conf.h"

#if defined(HASP_USE_I2C_GPIO) && defined(ARDUINO)

#include "sys/gpio/i2c_gpio_bus.h"

i2c_gpio_bus* i2c_gpio = nullptr;

bool i2c_gpio_bus::analogWrite8(uint8_t pin, uint8_t value)
{
    (void)pin;
    (void)value;
    return false;
}

int i2c_gpio_bus::analogRead8(uint8_t pin)
{
    (void)pin;
    return -1;
}

#endif

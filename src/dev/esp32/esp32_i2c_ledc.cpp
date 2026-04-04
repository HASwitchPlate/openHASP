/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "hasp_conf.h"

#if defined(ESP32) && defined(HASP_ESP32_DEVICE_I2C_BACKLIGHT) && defined(HASP_USE_I2C_GPIO) \
    && defined(ARDUINO_ARCH_ESP32)

#include <Arduino.h>
#include <Wire.h>

#include "esp32_i2c_ledc.h"
#include "hasp_debug.h"
#include "sys/gpio/i2c_gpio_bus.h"
#if defined(EXPANDER_ADDRESS) && defined(HASP_EXPANDER_GPIO_STC_L10)
#include "drv/gpio/i2c_gpio_stc_l10.h"
#endif

namespace dev {

void Esp32_I2c_ledc::update_backlight()
{
    uint8_t pin = get_backlight_pin();
#if defined(EXPANDER_BACKLIGHT_CHANNEL)
    if(pin == 0u || pin == 255u) pin = (uint8_t)EXPANDER_BACKLIGHT_CHANNEL;
#endif
    const bool pin_ok = (pin != 0u && pin != 255u);
    const bool i2c_ready = pin_ok && i2c_gpio && i2c_gpio->handles_pin(pin);

    const bool power  = Esp32Device::get_backlight_power();
    const uint8_t blv = Esp32Device::get_backlight_level();
    uint32_t duty     = power ? (uint32_t)map((long)blv, 0L, 255L, 0L, 1023L) : 0U;
    if(Esp32Device::get_backlight_invert()) duty = 1023U - duty;
    const uint8_t out = (uint8_t)map((long)duty, 0L, 1023L, 0L, 255L);

    if(i2c_ready) {
        (void)i2c_gpio->analogWrite8(pin, out);
        return;
    }
    else{
        #if defined(EXPANDER_ADDRESS) 
            /* Global i2c_gpio not ready yet (gpioSetup later); one-shot STC driver still reaches BL on shared Wire. */
            LOG_WARNING(TAG_LEDC, F("LEDI2C i2c_gpio-->handles_pin()=>%u is not ready"), (unsigned)pin);
            i2c_gpio_stc_l10 local_exp((uint8_t)EXPANDER_ADDRESS, Wire);
            const bool wr = local_exp.analogWrite8(pin, out);
            LOG_INFO(TAG_LEDC, F("LEDI2C i2c_gpio-->analogWrite8(%u, %u) after init=>%d"), (unsigned)pin, (unsigned)out,
                        wr ? 1 : 0);
        #else
            LOG_WARNING(TAG_LEDC, F("LEDI2C not ready (pin %u)"), (unsigned)pin);
        #endif
        return;
    }

    Esp32Device::update_backlight();
}

} // namespace dev

dev::Esp32_I2c_ledc haspDevice;

#endif

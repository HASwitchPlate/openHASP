/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_ESP32_I2C_LEDC_H
#define HASP_ESP32_I2C_LEDC_H

#if defined(ESP32) && defined(HASP_ESP32_DEVICE_I2C_BACKLIGHT) && defined(HASP_USE_I2C_GPIO)

#include "esp32.h"

namespace dev {

/**
 * ESP32 device with LCD backlight on an I2C virtual GPIO (via global @c i2c_gpio).
 * Uses @c get_backlight_pin() for UI changes; values @c 0 and @c 255 are treated as unset (GPIO0 / out-of-range) and
 * replaced by @c EXPANDER_BACKLIGHT_CHANNEL when defined. Uses @c i2c_gpio_bus::handles_pin() and @c analogWrite8().
 * For MCU GPIO backlights, falls back to @c Esp32Device LEDC behavior.
 */
class Esp32_I2c_ledc : public Esp32Device {

  protected:
    void update_backlight() override;
};

} // namespace dev

using dev::Esp32_I2c_ledc;
extern dev::Esp32_I2c_ledc haspDevice;

#endif /* ESP32 && I2C backlight device */

#endif /* HASP_ESP32_I2C_LEDC_H */

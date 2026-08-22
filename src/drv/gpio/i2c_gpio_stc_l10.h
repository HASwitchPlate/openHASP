/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   Lanbon L10 / STC-style I2C relay & backlight expander @ 0x20. */

#pragma once

#include "hasp_conf.h"

#if defined(HASP_USE_I2C_GPIO) && defined(ARDUINO) && defined(HASP_EXPANDER_GPIO_STC_L10)

#include "sys/gpio/i2c_gpio_bus.h"

class i2c_gpio_stc_l10 : public i2c_gpio_bus {
  public:
    explicit i2c_gpio_stc_l10(uint8_t i2c_addr, TwoWire& wire);

    bool begin() override;
    bool handles_pin(uint8_t pin) const override;
    bool pinMode(uint8_t pin, uint8_t mode) override;
    bool digitalWrite(uint8_t pin, uint8_t val) override;
    int digitalRead(uint8_t pin) override;
    bool digitalReadBytes(uint8_t pin, uint8_t* val, uint8_t read_len) override;
    bool analogWrite8(uint8_t pin, uint8_t value) override;
    int analogRead8(uint8_t pin) override;

  private:
    int digitalRead_internal(const i2c_gpio_pin_map_t* m);
    int analogReadAndCompare(const i2c_gpio_pin_map_t* m, uint8_t pin);

    const i2c_gpio_pin_map_t* find_map(uint8_t pin) const;
    bool pre_write(const i2c_gpio_pin_map_t* m);
    bool read_reg(uint8_t reg, uint8_t& val);
    bool write_reg(const i2c_gpio_pin_map_t* m, uint8_t reg, uint8_t val);
    bool writepointer_read(uint8_t reg, uint8_t* val, uint8_t pre_read_len);

    uint8_t _addr;
    TwoWire* _wire;
};

#endif

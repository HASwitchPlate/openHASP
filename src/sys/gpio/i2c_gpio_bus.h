/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   Generic I2C-backed virtual GPIO (expander protocols). */

#pragma once

#include "hasp_conf.h"

#if defined(HASP_USE_I2C_GPIO) && defined(ARDUINO)

#include <Arduino.h>
#include <Wire.h>
#include <stdint.h>

/** Per-pin map for STC/L10-style transactions (read pointer, write reg, suffix bytes). */
typedef struct {
    uint8_t pin;
    uint8_t read_reg;
    uint8_t read_pl_byte;
    bool isWPR;
    /** 1-based byte index in read buffer; 0 or 0xFF = last byte. */
    uint8_t read_pin_byte;
    uint8_t pre_read_len;
    uint8_t write_reg;
    uint8_t init_reg;
    uint8_t bit;
    uint8_t pre_write[4];
    uint8_t pre_write_len;
    uint8_t write_seq[8];
    uint8_t write_seq_len;
    uint8_t post_write[4];
    uint8_t post_write_len;
    /** If true, digitalRead() uses analogRead8 byte vs on_value/off_value (1/0/-1). */
    bool is_digital;
    uint8_t on_value;
    uint8_t off_value;
} i2c_gpio_pin_map_t;

class i2c_gpio_bus {
  public:
    virtual ~i2c_gpio_bus() {}
    virtual bool begin()                         = 0;
    virtual bool handles_pin(uint8_t pin) const = 0;
    virtual bool pinMode(uint8_t pin, uint8_t mode) = 0;
    virtual bool digitalWrite(uint8_t pin, uint8_t val) = 0;
    virtual int digitalRead(uint8_t pin)       = 0;
    virtual bool digitalReadBytes(uint8_t pin, uint8_t* val, uint8_t read_len) = 0;
    virtual bool analogWrite8(uint8_t pin, uint8_t value);
    virtual int analogRead8(uint8_t pin);
};

/** Global expander instance (set in gpioSetup). */
extern i2c_gpio_bus* i2c_gpio;

#endif /* HASP_USE_I2C_GPIO && ARDUINO */

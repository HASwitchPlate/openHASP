/*----------------------------------------------------------------------------/
  Lovyan GFX - Graphics library for embedded devices.

Original Source:
 https://github.com/lovyan03/LovyanGFX/

Licence:
 [FreeBSD](https://github.com/lovyan03/LovyanGFX/blob/master/license.txt)

Author:
 [lovyan03](https://twitter.com/lovyan03)

Contributors:
 [ciniml](https://github.com/ciniml)
 [mongonta0716](https://github.com/mongonta0716)
 [tobozo](https://github.com/tobozo)
/----------------------------------------------------------------------------*/
#pragma once

#if defined(ARDUINO) && defined(LGFX_USE_V1) && !defined(CONFIG_IDF_TARGET_ESP32C3)

#include "Arduino.h"
#include "LovyanGFX.hpp"
#include <driver/i2c.h>

namespace dev {

namespace m5stack {
static constexpr int32_t axp_i2c_freq      = 400000;
static constexpr uint_fast8_t axp_i2c_addr = 0x34;
static constexpr int_fast16_t axp_i2c_port = I2C_NUM_1;
static constexpr int_fast16_t axp_i2c_sda  = 21;
static constexpr int_fast16_t axp_i2c_scl  = 22;
} // namespace m5stack

struct Panel_M5Stack : public lgfx::Panel_ILI9342
{
    Panel_M5Stack(void)
    {
        _cfg.pin_cs          = 14;
        _cfg.pin_rst         = 33;
        _cfg.offset_rotation = 3;

        _rotation = 1;
    }

    bool init(bool use_reset) override
    {
        lgfx::gpio_hi(_cfg.pin_rst);
        lgfx::pinMode(_cfg.pin_rst, lgfx::pin_mode_t::input_pulldown);
        _cfg.invert = lgfx::gpio_in(_cfg.pin_rst); // get panel type (IPS or TN)
        lgfx::pinMode(_cfg.pin_rst, lgfx::pin_mode_t::output);

        return lgfx::Panel_ILI9342::init(use_reset);
    }
};

struct Panel_M5StackCore2 : public lgfx::Panel_ILI9342
{
    Panel_M5StackCore2(void)
    {
        _cfg.pin_cs          = 5;
        _cfg.invert          = true;
        _cfg.offset_rotation = 3;

        _rotation = 1; // default rotation
    }

    void reset(void)
    {
        using namespace m5stack;
        // AXP192 reg 0x96 = GPIO3&4 control
        lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x96, 0, ~0x02, axp_i2c_freq); // GPIO4 LOW (LCD RST)
        lgfx::delay(4);
        lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x96, 2, ~0x00, axp_i2c_freq); // GPIO4 HIGH (LCD RST)
    }
};

struct Light_M5StackCore2 : public lgfx::ILight
{
    bool init(uint8_t brightness) override
    {
        setBrightness(brightness);
        return true;
    }

    void setBrightness(uint8_t brightness) override
    {
        using namespace m5stack;

        if(brightness) {
            brightness = (brightness >> 3) + 72;
            lgfx::i2c::bitOn(axp_i2c_port, axp_i2c_addr, 0x12, 0x02, axp_i2c_freq); // DC3 enable
        } else {
            lgfx::i2c::bitOff(axp_i2c_port, axp_i2c_addr, 0x12, 0x02, axp_i2c_freq); // DC3 disable
        }
        // AXP192 reg 0x27 = DC3
        lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x27, brightness, 0x80, axp_i2c_freq);
    }
};

struct Light_M5Tough : public lgfx::ILight
{
    bool init(uint8_t brightness) override
    {
        setBrightness(brightness);
        return true;
    }

    void setBrightness(uint8_t brightness) override
    {
        using namespace m5stack;

        if(brightness) {
            if(brightness > 4) {
                brightness = (brightness / 24) + 5;
            }
            lgfx::i2c::bitOn(axp_i2c_port, axp_i2c_addr, 0x12, 0x08, axp_i2c_freq); // LDO3 enable
        } else {
            lgfx::i2c::bitOff(axp_i2c_port, axp_i2c_addr, 0x12, 0x08, axp_i2c_freq); // LDO3 disable
        }
        lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x28, brightness, 0xF0, axp_i2c_freq);
    }
};

struct Touch_M5Tough : public lgfx::ITouch
{
    Touch_M5Tough(void)
    {
        _cfg.x_min = 0;
        _cfg.x_max = 319;
        _cfg.y_min = 0;
        _cfg.y_max = 239;
    }

    void wakeup(void) override
    {}
    void sleep(void) override
    {}

    bool init(void) override
    {
        _inited = false;
        if(isSPI()) return false;

        if(_cfg.pin_int >= 0) {
            lgfx::pinMode(_cfg.pin_int, lgfx::v1::pin_mode_t::input_pullup);
        }
        _inited = lgfx::i2c::init(_cfg.i2c_port, _cfg.pin_sda, _cfg.pin_scl).has_value();
        static constexpr uint8_t irq_modechange_cmd[] = {0x5a, 0x5a}; /// (INT mode change)
        lgfx::i2c::transactionWrite(_cfg.i2c_port, _cfg.i2c_addr, irq_modechange_cmd, 2);
        return _inited;
    }

    uint_fast8_t getTouchRaw(lgfx::v1::touch_point_t* __restrict tp, uint_fast8_t count) override
    {
        if(tp) tp->size = 0;
        if(!_inited || count == 0) return 0;
        if(count > 2) count = 2; // max 2 point.

        if(_cfg.pin_int >= 0) {
            if(lgfx::v1::gpio_in(_cfg.pin_int)) Serial.print("#");
            if(lgfx::v1::gpio_in(_cfg.pin_int)) return 0;
        }

        size_t len = 3 + count * 6;
        uint8_t buf[2][len];
        int32_t retry = 5;
        bool flip     = false;
        uint8_t* tmp;
        for(;;) {
            tmp = buf[flip];
            memset(tmp, 0, len);
            if(lgfx::i2c::beginTransaction(_cfg.i2c_port, _cfg.i2c_addr, _cfg.freq, false)) {
                static constexpr uint8_t reg_number = 2;
                if(lgfx::i2c::writeBytes(_cfg.i2c_port, &reg_number, 1) &&
                   lgfx::i2c::restart(_cfg.i2c_port, _cfg.i2c_addr, _cfg.freq, true) &&
                   lgfx::i2c::readBytes(_cfg.i2c_port, tmp, 1) && (tmp[0] != 0)) {
                    flip          = !flip;
                    size_t points = std::min<uint_fast8_t>(count, tmp[0]);
                    if(points && lgfx::i2c::readBytes(_cfg.i2c_port, &tmp[1], points * 6 - 2)) {
                    }
                }
                if(lgfx::i2c::endTransaction(_cfg.i2c_port)) {
                }
                if(tmp[0] == 0 || memcmp(buf[0], buf[1], len) == 0) break;
            }

            if(0 == --retry) return 0;
        }
        if(count > tmp[0]) count = tmp[0];

        for(size_t idx = 0; idx < count; ++idx) {
            auto data    = &tmp[1 + idx * 6];
            tp[idx].size = 1;
            tp[idx].x    = (data[0] & 0x0F) << 8 | data[1];
            tp[idx].y    = (data[2] & 0x0F) << 8 | data[3];
            tp[idx].id   = idx;
        }
        return count;
    }
};

struct Panel_M5StickC : public lgfx::Panel_ST7735S
{
    Panel_M5StickC(void)
    {
        _cfg.invert          = true;
        _cfg.pin_cs          = 5;
        _cfg.pin_rst         = 18;
        _cfg.panel_width     = 80;
        _cfg.panel_height    = 160;
        _cfg.offset_x        = 26;
        _cfg.offset_y        = 1;
        _cfg.offset_rotation = 2;
    }

  protected:
    const uint8_t* getInitCommands(uint8_t listno) const override
    {
        static constexpr uint8_t list[] = {
            CMD_GAMMASET, 1,    0x08, // Gamma set, curve 4
            0xFF,         0xFF,       // end
        };
        if(listno == 2) return list;
        return Panel_ST7735S::getInitCommands(listno);
    }
};

struct Light_M5StickC : public lgfx::ILight
{
    bool init(uint8_t brightness) override
    {
        using namespace m5stack;
        lgfx::i2c::init(axp_i2c_port, axp_i2c_sda, axp_i2c_scl);
        lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x12, 0x4D, ~0, axp_i2c_freq);
        setBrightness(brightness);
        return true;
    }

    void setBrightness(uint8_t brightness) override
    {
        using namespace m5stack;
        if(brightness) {
            brightness = (((brightness >> 1) + 8) / 13) + 5;
            lgfx::i2c::bitOn(axp_i2c_port, axp_i2c_addr, 0x12, 1 << 2, axp_i2c_freq);
        } else {
            lgfx::i2c::bitOff(axp_i2c_port, axp_i2c_addr, 0x12, 1 << 2, axp_i2c_freq);
        }
        lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x28, brightness << 4, 0x0F, axp_i2c_freq);
    }
};

struct Panel_M5StickCPlus : public lgfx::Panel_ST7789
{
    Panel_M5StickCPlus(void)
    {
        _cfg.invert       = true;
        _cfg.pin_cs       = 5;
        _cfg.pin_rst      = 18;
        _cfg.panel_width  = 135;
        _cfg.panel_height = 240;
        _cfg.offset_x     = 52;
        _cfg.offset_y     = 40;
    }
};

} // namespace dev

#endif
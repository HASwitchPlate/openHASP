#include "Arduino_RGB_Display_mod.h"

#if defined(ESP32) && defined(CONFIG_IDF_TARGET_ESP32S3) && defined(HASP_USE_ARDUINOGFX)

#include "Arduino_GFX.h"
#include "Arduino_DataBus.h"
#include "Arduino_RGB_Display_mod.h"

namespace {

bool execute_custom_init_ops(Arduino_DataBus* bus, const uint8_t* ops, size_t len)
{
    if(!bus || !ops || len == 0) return true;

    size_t i = 0;
    while(i < len) {
        const uint8_t op = ops[i++];
        switch(op) {
            case BEGIN_WRITE:
                bus->beginWrite();
                break;
            case END_WRITE:
                bus->endWrite();
                break;
            case WRITE_COMMAND_8:
                if(i >= len) return false;
                bus->writeCommand(ops[i++]);
                break;
            case WRITE_C8_D8:
                if(i + 1 >= len) return false;
                bus->writeCommand(ops[i++]);
                bus->write(ops[i++]);
                break;
            case WRITE_C8_D16:
                if(i + 2 >= len) return false;
                bus->writeCommand(ops[i++]);
                bus->write(ops[i++]);
                bus->write(ops[i++]);
                break;
            case WRITE_BYTES: {
                if(i >= len) return false;
                uint8_t n = ops[i++];
                if(i + n > len) return false;
                for(uint8_t k = 0; k < n; k++) bus->write(ops[i++]);
                break;
            }
            case DELAY:
                if(i >= len) return false;
                delay(ops[i++]);
                break;
            case I2C_WRITE_REG:
                if(i + 1 >= len) return false;
#if defined(USE_I2C_SW_SPI)
                bus->writeRegisterI2COpcode(ops[i], ops[i + 1]);
                i += 2;
#else
                return false;
#endif
                break;
            case I2C_WRITE_SEQ:
                if(i + 1 >= len) return false;
                {
                    const uint8_t reg = ops[i++];
                    const uint8_t n = ops[i++];
                    if(i + n > len) return false;
#if defined(USE_I2C_SW_SPI)
                    bus->writeRegisterI2CSeqOpcode(reg, &ops[i], n);
                    i += n;
#else
                    return false;
#endif
                }
                break;
            case REPEAT_IN:
                if(i + 3 >= len) return false;
                {
                    const uint8_t count = ops[i++];
                    const uint8_t nested_op = ops[i++];
                    if(nested_op != I2C_WRITE_REG) return false;
                    const uint8_t reg = ops[i++];
                    const uint8_t val = ops[i++];
#if defined(USE_I2C_SW_SPI)
                    for(uint8_t r = 0; r < count; r++) bus->writeRegisterI2COpcode(reg, val);
#else
                    return false;
#endif
                }
                break;
            default:
                return false;
        }
    }

    return true;
}

} // namespace

Arduino_RGB_Display_Mod::Arduino_RGB_Display_Mod(int16_t w, int16_t h, Arduino_RGBPanel_Mod* rgbpanel, uint8_t r,
                                                 bool auto_flush, Arduino_DataBus* bus, int8_t rst,
                                                 const uint8_t* init_operations, size_t init_operations_len)
    : Arduino_GFX(w, h), _rgbpanel(rgbpanel), _auto_flush(auto_flush), _bus(bus), _rst(rst),
      _init_operations(init_operations), _init_operations_len(init_operations_len)
{
    _framebuffer_size = w * h * 2;
    _rotation         = r;
}

bool Arduino_RGB_Display_Mod::begin(int32_t speed)
{
    _rgbpanel->begin(speed);

    if(_bus) {
        _bus->begin();
    }

    if(_rst != GFX_NOT_DEFINED) {
        pinMode(_rst, OUTPUT);
        digitalWrite(_rst, HIGH);
        delay(100);
        digitalWrite(_rst, LOW);
        delay(120);
        digitalWrite(_rst, HIGH);
        delay(120);
    } else {
        if(_bus) {
            // Software Rest
            _bus->sendCommand(0x01);
            delay(120);
        }
    }

    if(_bus) {
        if(_init_operations_len > 0) {
            if(!execute_custom_init_ops(_bus, _init_operations, _init_operations_len)) {
                _bus->batchOperation((uint8_t*)_init_operations, _init_operations_len);
            }
        }
    }

    _framebuffer = _rgbpanel->getFrameBuffer(WIDTH, HEIGHT);

    if(!_framebuffer) {
        return false;
    }

    return true;
}

void Arduino_RGB_Display_Mod::writePixelPreclipped(int16_t x, int16_t y, uint16_t color)
{
    uint16_t* fb = _framebuffer;
    fb += (int32_t)y * _width;
    fb += x;
    *fb = color;
    if(_auto_flush) {
        Cache_WriteBack_Addr((uint32_t)fb, 2);
    }
}

void Arduino_RGB_Display_Mod::writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
    if(_ordered_in_range(x, 0, _max_x) && h) { // X on screen, nonzero height
        if(h < 0) {                            // If negative height...
            y += h + 1;                        //   Move Y to top edge
            h = -h;                            //   Use positive height
        }
        if(y <= _max_y) { // Not off bottom
            int16_t y2 = y + h - 1;
            if(y2 >= 0) { // Not off top
                // Line partly or fully overlaps screen
                if(y < 0) {
                    y = 0;
                    h = y2 + 1;
                } // Clip top
                if(y2 > _max_y) {
                    h = _max_y - y + 1;
                } // Clip bottom

                uint16_t* fb = _framebuffer + ((int32_t)y * _width) + x;
                if(_auto_flush) {
                    while(h--) {
                        *fb = color;
                        Cache_WriteBack_Addr((uint32_t)fb, 2);
                        fb += _width;
                    }
                } else {
                    while(h--) {
                        *fb = color;
                        fb += _width;
                    }
                }
            }
        }
    }
}

void Arduino_RGB_Display_Mod::writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
    if(_ordered_in_range(y, 0, _max_y) && w) { // Y on screen, nonzero width
        if(w < 0) {                            // If negative width...
            x += w + 1;                        //   Move X to left edge
            w = -w;                            //   Use positive width
        }
        if(x <= _max_x) { // Not off right
            int16_t x2 = x + w - 1;
            if(x2 >= 0) { // Not off left
                // Line partly or fully overlaps screen
                if(x < 0) {
                    x = 0;
                    w = x2 + 1;
                } // Clip left
                if(x2 > _max_x) {
                    w = _max_x - x + 1;
                } // Clip right

                uint16_t* fb      = _framebuffer + ((int32_t)y * _width) + x;
                uint32_t cachePos = (uint32_t)fb;
                int16_t writeSize = w * 2;
                while(w--) {
                    *(fb++) = color;
                }
                if(_auto_flush) {
                    Cache_WriteBack_Addr(cachePos, writeSize);
                }
            }
        }
    }
}

void Arduino_RGB_Display_Mod::writeFillRectPreclipped(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    uint16_t* row = _framebuffer;
    row += y * _width;
    uint32_t cachePos = (uint32_t)row;
    row += x;
    for(int j = 0; j < h; j++) {
        for(int i = 0; i < w; i++) {
            row[i] = color;
        }
        row += _width;
    }
    if(_auto_flush) {
        Cache_WriteBack_Addr(cachePos, _width * h * 2);
    }
}

void Arduino_RGB_Display_Mod::setRotation(uint8_t r)
{
    esp_err_t err = esp_lcd_panel_swap_xy(_rgbpanel->_panel_handle, r & 1);
    err           = esp_lcd_panel_mirror(_rgbpanel->_panel_handle, r & 4, r & 2);
}

void Arduino_RGB_Display_Mod::invertDisplay(bool i)
{
    esp_err_t err = esp_lcd_panel_invert_color(_rgbpanel->_panel_handle, i);
}

void Arduino_RGB_Display_Mod::draw16bitRGBBitmap(int16_t x, int16_t y, uint16_t* bitmap, int16_t w, int16_t h)
{
    esp_err_t err = esp_lcd_panel_draw_bitmap(_rgbpanel->_panel_handle, x, y, x + w, y + h, bitmap);
    return;

    if(((x + w - 1) < 0) || // Outside left
       ((y + h - 1) < 0) || // Outside top
       (x > _max_x) ||      // Outside right
       (y > _max_y)         // Outside bottom
    ) {
        return;
    } else {
        int16_t xskip = 0;
        if((y + h - 1) > _max_y) {
            h -= (y + h - 1) - _max_y;
        }
        if(y < 0) {
            bitmap -= y * w;
            h += y;
            y = 0;
        }
        if((x + w - 1) > _max_x) {
            xskip = (x + w - 1) - _max_x;
            w -= xskip;
        }
        if(x < 0) {
            bitmap -= x;
            xskip -= x;
            w += x;
            x = 0;
        }
        uint16_t* row = _framebuffer;
        row += y * _width;
        uint32_t cachePos = (uint32_t)row;
        row += x;
        if(((_width & 1) == 0) && ((xskip & 1) == 0) && ((w & 1) == 0)) {
            uint32_t* row2    = (uint32_t*)row;
            uint32_t* bitmap2 = (uint32_t*)bitmap;
            int16_t _width2   = _width >> 1;
            int16_t xskip2    = xskip >> 1;
            int16_t w2        = w >> 1;

            for(int16_t j = 0; j < h; j++) {
                for(int16_t i = 0; i < w2; i++) {
                    row2[i] = *bitmap2++;
                }
                bitmap2 += xskip2;
                row2 += _width2;
            }
        } else {
            for(int j = 0; j < h; j++) {
                for(int i = 0; i < w; i++) {
                    row[i] = *bitmap++;
                }
                bitmap += xskip;
                row += _width;
            }
        }
        if(_auto_flush) {
            Cache_WriteBack_Addr(cachePos, _width * h * 2);
        }
    }
}

void Arduino_RGB_Display_Mod::draw16bitBeRGBBitmap(int16_t x, int16_t y, uint16_t* bitmap, int16_t w, int16_t h)
{
    if(((x + w - 1) < 0) || // Outside left
       ((y + h - 1) < 0) || // Outside top
       (x > _max_x) ||      // Outside right
       (y > _max_y)         // Outside bottom
    ) {
        return;
    } else {
        int16_t xskip = 0;
        if((y + h - 1) > _max_y) {
            h -= (y + h - 1) - _max_y;
        }
        if(y < 0) {
            bitmap -= y * w;
            h += y;
            y = 0;
        }
        if((x + w - 1) > _max_x) {
            xskip = (x + w - 1) - _max_x;
            w -= xskip;
        }
        if(x < 0) {
            bitmap -= x;
            xskip -= x;
            w += x;
            x = 0;
        }
        uint16_t* row = _framebuffer;
        row += y * _width;
        uint32_t cachePos = (uint32_t)row;
        row += x;
        uint16_t color;
        for(int j = 0; j < h; j++) {
            for(int i = 0; i < w; i++) {
                color = *bitmap++;
                MSB_16_SET(row[i], color);
            }
            bitmap += xskip;
            row += _width;
        }
        if(_auto_flush) {
            Cache_WriteBack_Addr(cachePos, _width * h * 2);
        }
    }
}

void Arduino_RGB_Display_Mod::flush(void)
{
    if(!_auto_flush) {
        Cache_WriteBack_Addr((uint32_t)_framebuffer, _framebuffer_size);
    }
}

uint16_t* Arduino_RGB_Display_Mod::getFramebuffer()
{
    return _framebuffer;
}

#endif // #if defined(ESP32) && (CONFIG_IDF_TARGET_ESP32S3)

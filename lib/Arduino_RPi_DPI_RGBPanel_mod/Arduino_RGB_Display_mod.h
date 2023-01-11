#ifdef HASP_USE_ARDUINOGFX
#include "Arduino.h"
#include "Arduino_DataBus.h"
#endif

#if defined(ESP32) && (CONFIG_IDF_TARGET_ESP32S3) && defined(HASP_USE_ARDUINOGFX)

#ifndef _ARDUINO_RGB_DISPLAY_MOD_H_
#define _ARDUINO_RGB_DISPLAY_MOD_H_

#include "Arduino_GFX.h"
#include "Arduino_RGBPanel_mod.h"

class Arduino_RGB_Display_Mod : public Arduino_GFX{
public:
  Arduino_RGB_Display_Mod(
      int16_t w, int16_t h, Arduino_RGBPanel_Mod *rgbpanel, uint8_t r = 0, bool auto_flush = true,
      Arduino_DataBus *bus = NULL, int8_t rst = GFX_NOT_DEFINED, const uint8_t *init_operations = NULL, size_t init_operations_len = GFX_NOT_DEFINED);

  bool begin(int32_t speed = GFX_NOT_DEFINED) override;
   void writePixelPreclipped(int16_t x, int16_t y, uint16_t color) override;
   void writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) override;
   void writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) override;
   void writeFillRectPreclipped(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;
   void draw16bitRGBBitmap(int16_t x, int16_t y, uint16_t *bitmap, int16_t w, int16_t h) override;
   void draw16bitBeRGBBitmap(int16_t x, int16_t y, uint16_t *bitmap, int16_t w, int16_t h) override;
  void flush(void) override;

    void setRotation(uint8_t r);
    void invertDisplay(bool i);

  uint16_t *getFramebuffer();
  Arduino_DataBus *_bus;
  Arduino_RGBPanel_Mod *_rgbpanel;

protected:
  uint16_t *_framebuffer;
  size_t _framebuffer_size;
  bool _auto_flush;
  int8_t _rst;
  const uint8_t *_init_operations;
  size_t _init_operations_len;

private:
};

#endif // _ARDUINO_RGB_DISPLAY_H_

#endif // #if defined(ESP32) && (CONFIG_IDF_TARGET_ESP32S3)

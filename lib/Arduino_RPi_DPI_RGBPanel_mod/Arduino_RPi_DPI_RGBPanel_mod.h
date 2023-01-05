#if defined(ESP32) && (CONFIG_IDF_TARGET_ESP32S3) && defined(HASP_USE_ARDUINOGFX)

#include "Arduino_DataBus.h"

#ifndef _ARDUINO_RPI_DPI_RGBPANEL_MOD_H_
#define _ARDUINO_RPI_DPI_RGBPANEL_MOD_H_

#include "Arduino_GFX.h"
#include "Arduino_ESP32RGBPanel_mod.h"

class Arduino_RGBPanel_Mod : public Arduino_GFX
{
public:
  Arduino_RGBPanel_Mod(
      Arduino_ESP32RGBPanel_Mod *bus,
      int16_t w, uint16_t hsync_polarity, uint16_t hsync_front_porch, uint16_t hsync_pulse_width, uint16_t hsync_back_porch,
      int16_t h, uint16_t vsync_polarity, uint16_t vsync_front_porch, uint16_t vsync_pulse_width, uint16_t vsync_back_porch,
      uint16_t pclk_active_neg = 0, int32_t prefer_speed = GFX_NOT_DEFINED, bool auto_flush = true);

  void begin(int32_t speed = GFX_NOT_DEFINED) override;
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
  Arduino_ESP32RGBPanel_Mod *_bus;

protected:
  uint16_t *_framebuffer;
  size_t _framebuffer_size;

  uint16_t _hsync_polarity;
  uint16_t _hsync_front_porch;
  uint16_t _hsync_pulse_width;
  uint16_t _hsync_back_porch;
  uint16_t _vsync_polarity;
  uint16_t _vsync_front_porch;
  uint16_t _vsync_pulse_width;
  uint16_t _vsync_back_porch;
  uint16_t _pclk_active_neg;
  int32_t _prefer_speed;
  bool _auto_flush;

private:
};

#endif // _ARDUINO_RPI_DPI_RGBPANEL_H_

#endif // #if defined(ESP32) && (CONFIG_IDF_TARGET_ESP32S3)

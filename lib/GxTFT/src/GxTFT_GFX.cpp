/*
  See rights and use declaration in License.h
  This library has been modified for the Maple Mini.
  Includes DMA transfers on DMA1 CH2 and CH3.
*/

// modified by Jean-Marc Zingg to be the GxTFT_GFX class for the GxTFT library
// (the "light" display class using Adafruit_GFX for graphics)
// original source taken from https://github.com/stevstrong/Adafruit_ILI9486_STM32/tree/master

#include "GxTFT_GFX.h"

// Swap any type
template <typename T> static inline void
swap(T& a, T& b)
{
  T t = a;
  a = b;
  b = t;
}

/*****************************************************************************/
// Constructor uses GxIO, the pins being specific to each device
/*****************************************************************************/
GxTFT_GFX::GxTFT_GFX(GxIO& io, GxCTRL& controller, uint16_t w, uint16_t h) : Adafruit_GFX(w, h), IO(io), Controller(controller)
{
  _initial_rotation = w > h;
  // setRotation expects _tft_width, _tft_height in portrait orientation
  _tft_width = min(w, h);  // portrait width
  _tft_height = max(w, h); // portrait height
}
/*****************************************************************************/
void GxTFT_GFX::init()
{
  IO.init();
  Controller.init(); // may choose whatever is its default orientation
  // set orientation according to constructor dimensions
  setRotation(_initial_rotation);
  IO.setBackLight(true);
}
/*****************************************************************************/
void GxTFT_GFX::setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
  Controller.setWindow(x0, y0, x1, y1);
}
/*****************************************************************************/
void GxTFT_GFX::pushColor(uint16_t color)
{
  IO.writeData16(color);
}
/*****************************************************************************/
void GxTFT_GFX::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if ((x < 0) || (x >= _width) || (y < 0) || (y >= _height)) return;
  return Controller.drawPixel(x, y, color);
  IO.startTransaction();
  Controller.setWindowAddress(x, y, x, y);
  IO.writeData16(color);
  IO.endTransaction();
}
/*****************************************************************************/
void GxTFT_GFX::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
  // Rudimentary clipping
  if ((x >= _width) || (y >= _height || h < 1)) return;
  if ((y + h - 1) >= _height)
  {
    h = _height - y;
  }
  return Controller.drawLine(x, y, x, y + h - 1, color);
  IO.startTransaction();
  Controller.setWindowAddress(x, y, x, y + h - 1);
  IO.writeData16(color, h);
  IO.endTransaction();
}
/*****************************************************************************/
void GxTFT_GFX::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
  // Rudimentary clipping
  if ((x >= _width) || (y >= _height || w < 1)) return;
  if ((x + w - 1) >= _width)
  {
    w = _width - x;
  }
  return Controller.drawLine(x, y, x + w - 1, y, color);
  IO.startTransaction();
  Controller.setWindowAddress(x, y, x + w - 1, y);
  IO.writeData16(color, w);
  IO.endTransaction();
}

/*****************************************************************************/
void GxTFT_GFX::fillScreen(uint16_t color)
{
  return Controller.fillRect(0, 0, _width, _height, color);
  IO.startTransaction();
  Controller.setWindowAddress(0, 0, _width - 1, _height - 1);
  IO.writeData16(color, uint32_t(_width) * uint32_t(_height));
  IO.endTransaction();
}

/*****************************************************************************/
void GxTFT_GFX::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  // rudimentary clipping (drawChar w/big text requires this)
  if ((x >= _width) || (y >= _height || h < 1 || w < 1)) return;
  if ((x + w - 1) >= _width)
  {
    w = _width  - x;
  }
  if ((y + h - 1) >= _height)
  {
    h = _height - y;
  }
  return Controller.fillRect(x, y, w, h, color);
  IO.startTransaction();
  Controller.setWindowAddress(x, y, x + w - 1, y + h - 1);
  IO.writeData16(color, uint32_t(w) * uint32_t(h));
  IO.endTransaction();
}

/*****************************************************************************/
void GxTFT_GFX::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
  if ((y0 < 0 && y1 < 0) || (y0 > _height && y1 > _height)) return;
  if ((x0 < 0 && x1 < 0) || (x0 > _width && x1 > _width)) return;
  if (x0 < 0) x0 = 0;
  if (x1 < 0) x1 = 0;
  if (y0 < 0) y0 = 0;
  if (y1 < 0) y1 = 0;
  return Controller.drawLine(x0, y0, x1, y1, color);
}

/*****************************************************************************/
// Pass 8-bit (each) R,G,B, get back 16-bit packed color
/*****************************************************************************/
uint16_t GxTFT_GFX::color565(uint8_t r, uint8_t g, uint8_t b)
{
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

/*****************************************************************************/
void GxTFT_GFX::setRotation(uint8_t r)
{
  Controller.setRotation(r);
  switch (r & 3)
  {
    case 0:
      _width  = _tft_width;
      _height = _tft_height;
      break;
    case 1:
      _width  = _tft_height;
      _height = _tft_width;
      break;
    case 2:
      _width  = _tft_width;
      _height = _tft_height;
      break;
    case 3:
      _width  = _tft_height;
      _height = _tft_width;
      break;
  }
}

/*****************************************************************************/
void GxTFT_GFX::invertDisplay(boolean i)
{
  Controller.invertDisplay(i);
}

void GxTFT_GFX::drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
  if (Controller.drawCircle(x0, y0, r, color)) return;
  Adafruit_GFX::drawCircle(x0, y0, r, color);
}

void GxTFT_GFX::fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
  if (Controller.fillCircle(x0, y0, r, color)) return;
  Adafruit_GFX::fillCircle(x0, y0, r, color);
}

void GxTFT_GFX::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
  if (Controller.drawTriangle(x0, y0, x1, y1, x2, y2, color)) return;
  Adafruit_GFX::drawTriangle(x0, y0, x1, y1, x2, y2, color);
}

void GxTFT_GFX::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
  if (Controller.fillTriangle(x0, y0, x1, y1, x2, y2, color)) return;
  Adafruit_GFX::fillTriangle(x0, y0, x1, y1, x2, y2, color);
}

void GxTFT_GFX::drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t radius, uint16_t color)
{
  if (Controller.drawRoundRect(x, y, w, h, radius, color)) return;
  Adafruit_GFX::drawRoundRect(x, y, w, h, radius, color);
}

void GxTFT_GFX::fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t radius, uint16_t color)
{
  if (Controller.fillRoundRect(x, y, w, h, radius, color)) return;
  Adafruit_GFX::fillRoundRect(x, y, w, h, radius, color);
}

void GxTFT_GFX::drawEllipse(int16_t x0, int16_t y0, int16_t rx, int16_t ry, uint16_t color)
{
  if (Controller.drawEllipse(x0, y0, rx, ry, color)) return;
  //Adafruit_GFX::drawEllipse(x0, y0, rx, ry, color);
}

void GxTFT_GFX::fillEllipse(int16_t x0, int16_t y0, int16_t rx, int16_t ry, uint16_t color)
{
  if (Controller.fillEllipse(x0, y0, rx, ry, color)) return;
  //Adafruit_GFX::fillEllipse(x0, y0, rx, ry, color);
}

void GxTFT_GFX::drawCurve(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint8_t curvePart, uint16_t color)
{
  if (Controller.drawCurve(xCenter, yCenter, longAxis, shortAxis, curvePart, color)) return;
}

void GxTFT_GFX::fillCurve(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint8_t curvePart, uint16_t color)
{
  if (Controller.fillCurve(xCenter, yCenter, longAxis, shortAxis, curvePart, color)) return;
}


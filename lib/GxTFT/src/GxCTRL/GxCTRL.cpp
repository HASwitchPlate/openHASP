// created by Jean-Marc Zingg to be the GxCTRL base class for the GxTFT library
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE

#include "GxCTRL.h"

// Swap any type
template <typename T> static inline void
swap(T& a, T& b) { T t = a; a = b; b = t; }

void GxCTRL::setWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
  IO.startTransaction();
  setWindowAddress(x0, y0, x1, y1);
  IO.endTransaction();
}

void GxCTRL::drawPixel(uint16_t x, uint16_t y, uint16_t color)
{
  // no range check here, is callers obligation
  IO.startTransaction();
  setWindowAddress(x, y, x, y);
  IO.writeData16(color);
  IO.endTransaction();
}

void GxCTRL::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
  IO.startTransaction();
  boolean steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep)
  {
    swap(x0, y0);
    swap(x1, y1);
  }
  if (x0 > x1)
  {
    swap(x0, x1);
    swap(y0, y1);
  }
  int16_t dx = x1 - x0, dy = abs(y1 - y0);;
  int16_t err = dx >> 1, ystep = -1, xs = x0, dlen = 0;
  if (y0 < y1) ystep = 1;

  // Split into steep and not steep for FastH/V separation
  if (steep)
  {
    for (; x0 <= x1; x0++)
    {
      dlen++;
      err -= dy;
      if (err < 0)
      {
        err += dx;
        setWindowAddress(y0, xs, y0, x1);
        IO.writeData16(color, dlen); dlen = 0;
        y0 += ystep; xs = x0 + 1;
      }
    }
    if (dlen)
    {
      // drawFastVLine(y0, xs, dlen, color);
      setWindowAddress(y0, xs, y0, xs + dlen - 1);
      IO.writeData16(color, dlen);
    }
  }
  else
  {
    for (; x0 <= x1; x0++)
    {
      dlen++;
      err -= dy;
      if (err < 0)
      {
        err += dx;
        setWindowAddress(xs, y0, x1, y0);
        IO.writeData16(color, dlen); dlen = 0;
        y0 += ystep; xs = x0 + 1;
      }
    }
    if (dlen)
    {
      // drawFastHLine(xs, y0, dlen, color);
      setWindowAddress(xs, y0, xs + dlen - 1, y0);
      IO.writeData16(color, dlen);
      IO.endTransaction();
    }
  }
  IO.endTransaction();
}

void GxCTRL::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  IO.startTransaction();
  // drawFastHLine(x, y, w, color);
  setWindowAddress(x, y, x + w - 1, y);
  IO.writeData16(color, w);
  // drawFastHLine(x, y + h - 1, w, color);
  setWindowAddress(x, y + h - 1, x + w - 1, y + h - 1);
  IO.writeData16(color, w);
  // drawFastVLine(x, y, h, color);
  setWindowAddress(x, y, x, y + h - 1);
  IO.writeData16(color, h);
  // drawFastVLine(x + w - 1, y, h, color);
  setWindowAddress(x + w - 1, y, x + w - 1, y + h - 1);
  IO.writeData16(color, h);
  IO.endTransaction();
}

void GxCTRL::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  IO.startTransaction();
  setWindowAddress(x, y, x + w - 1, y + h - 1);
  IO.writeData16(color, uint32_t(w) * uint32_t(h));
  IO.endTransaction();
}

void GxCTRL::invertDisplay(bool i)
{
  IO.writeCommandTransaction(i ? 0x21 : 0x20);
}


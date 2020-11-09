// created by Jean-Marc Zingg to be the GxCTRL_ILI9481 class for the GxTFT library
// code extracts taken from https://github.com/Bodmer/TFT_HX8357
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE

#include "GxCTRL_ILI9481.h"

#define CASET 0x2A
#define PASET 0x2B
#define RAMWR 0x2C
#define RAMRD 0x2E
#define MADCTL 0x36
#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_SS  0x02
#define MADCTL_GS  0x01

uint32_t GxCTRL_ILI9481::readID()
{
  return readRegister(0xBF, 2, 2);
}

uint32_t GxCTRL_ILI9481::readRegister(uint8_t nr, uint8_t index, uint8_t bytes)
{
  uint32_t rv = 0;
  bytes = min(bytes, 4);
  IO.startTransaction();
  IO.writeCommand(nr);
  IO.readData(); // dummy
  for (uint8_t i = 0; i < index; i++)
  {
    IO.readData(); // skip
  }
  for (; bytes > 0; bytes--)
  {
    rv <<= 8;
    rv |= IO.readData();
  }
  IO.endTransaction();
  return rv;
}

uint16_t GxCTRL_ILI9481::readPixel(uint16_t x, uint16_t y)
{
  uint16_t rv;
  readRect(x, y, 1, 1, &rv);
  return rv;
}

void GxCTRL_ILI9481::readRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t* data)
{
  uint16_t xe = x + w - 1;
  uint16_t ye = y + h - 1;
  uint32_t num = uint32_t(w) * uint32_t(h);
  IO.startTransaction();
  IO.writeCommand(CASET);  // Column addr set
  IO.writeData(x >> 8);
  IO.writeData(x & 0xFF);  // XSTART
  IO.writeData(xe >> 8);
  IO.writeData(xe & 0xFF); // XEND
  IO.writeCommand(PASET);  // Row addr set
  IO.writeData(y >> 8);
  IO.writeData(y);         // YSTART
  IO.writeData(ye >> 8);
  IO.writeData(ye);        // YEND
  IO.writeCommand(RAMRD);  // read from RAM
  IO.readData(); // dummy
  for (; num > 0; num--)
  {
    uint16_t d = IO.readData16();
    *data++ = ((d & 0x001F) << 11) | (d & 0x07E0) | ((d & 0xF800) >> 11); // r,b swapped
  }
  IO.endTransaction();
}

void GxCTRL_ILI9481::init()
{
  // Configure ILI9481 display

  IO.writeCommandTransaction(0x11);
  delay(20);
  IO.writeCommandTransaction(0xD0);
  IO.writeDataTransaction(0x07);
  IO.writeDataTransaction(0x42);
  IO.writeDataTransaction(0x18);

  IO.writeCommandTransaction(0xD1);
  IO.writeDataTransaction(0x00);
  IO.writeDataTransaction(0x07);
  IO.writeDataTransaction(0x10);

  IO.writeCommandTransaction(0xD2);
  IO.writeDataTransaction(0x01);
  IO.writeDataTransaction(0x02);

  IO.writeCommandTransaction(0xC0);
  IO.writeDataTransaction(0x10);
  IO.writeDataTransaction(0x3B);
  IO.writeDataTransaction(0x00);
  IO.writeDataTransaction(0x02);
  IO.writeDataTransaction(0x11);

  IO.writeCommandTransaction(0xC5);
  IO.writeDataTransaction(0x03);

  IO.writeCommandTransaction(0xC8);
  IO.writeDataTransaction(0x00);
  IO.writeDataTransaction(0x32);
  IO.writeDataTransaction(0x36);
  IO.writeDataTransaction(0x45);
  IO.writeDataTransaction(0x06);
  IO.writeDataTransaction(0x16);
  IO.writeDataTransaction(0x37);
  IO.writeDataTransaction(0x75);
  IO.writeDataTransaction(0x77);
  IO.writeDataTransaction(0x54);
  IO.writeDataTransaction(0x0C);
  IO.writeDataTransaction(0x00);

  IO.writeCommandTransaction(0x36);
  IO.writeDataTransaction(0x0A);

  IO.writeCommandTransaction(0x3A);
  IO.writeDataTransaction(0x55);

  IO.writeCommandTransaction(0x2A);
  IO.writeDataTransaction(0x00);
  IO.writeDataTransaction(0x00);
  IO.writeDataTransaction(0x01);
  IO.writeDataTransaction(0x3F);

  IO.writeCommandTransaction(0x2B);
  IO.writeDataTransaction(0x00);
  IO.writeDataTransaction(0x00);
  IO.writeDataTransaction(0x01);
  IO.writeDataTransaction(0xDF);

  delay(120);
  IO.writeCommandTransaction(0x29);

  delay(25);
  // End of ILI9481 display configuration
}

void GxCTRL_ILI9481::setWindowAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
  IO.writeCommand(CASET);  // Column addr set
  IO.writeData(x0 >> 8);
  IO.writeData(x0 & 0xFF); // XSTART
  IO.writeData(x1 >> 8);
  IO.writeData(x1 & 0xFF); // XEND
  IO.writeCommand(PASET);  // Row addr set
  IO.writeData(y0 >> 8);
  IO.writeData(y0);        // YSTART
  IO.writeData(y1 >> 8);
  IO.writeData(y1);        // YEND
  IO.writeCommand(RAMWR);  // write to RAM
}

void GxCTRL_ILI9481::setRotation(uint8_t r)
{
  IO.startTransaction();
  IO.writeCommand(MADCTL);
  switch (r & 3)
  {
    case 0: // Portrait
      IO.writeData(MADCTL_BGR | MADCTL_SS);
      break;
    case 1: // Landscape (Portrait + 90)
      IO.writeData(MADCTL_MV | MADCTL_BGR);
      break;
    case 2: // Inverter portrait
      IO.writeData( MADCTL_BGR | MADCTL_GS);
      break;
    case 3: // Inverted landscape
      IO.writeData(MADCTL_MV | MADCTL_BGR | MADCTL_SS | MADCTL_GS);
      break;
  }
  IO.endTransaction();
}


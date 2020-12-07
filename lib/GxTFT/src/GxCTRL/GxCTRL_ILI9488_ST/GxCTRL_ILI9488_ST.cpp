// created by Jean-Marc Zingg to be the GxCTRL_ILI9488_ST class for the GxTFT library
// init register values taken from MCUFRIEND_kbv
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// GxCTRL_ILI9488_ST : Separate Transactions, transfers with individual CS activations
//                     some SPI ili9486 TFTs needed this

#include "GxCTRL_ILI9488_ST.h"

#define ILI9488_CASET 0x2A
#define ILI9488_PASET 0x2B
#define ILI9488_RAMWR 0x2C
#define ILI9488_RAMRD 0x2E
#define ILI9488_MADCTL 0x36
#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_BGR 0x08
#define MADCTL_MH  0x04

uint32_t GxCTRL_ILI9488_ST::readID()
{
  return readRegister(0xD3, 0, 3);
}

uint32_t GxCTRL_ILI9488_ST::readRegister(uint8_t nr, uint8_t index, uint8_t bytes)
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

uint16_t GxCTRL_ILI9488_ST::readPixel(uint16_t x, uint16_t y)
{
  uint16_t rv;
  readRect(x, y, 1, 1, &rv);
  return rv;
}

void GxCTRL_ILI9488_ST::readRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t* data)
{
  uint16_t xe = x + w - 1;
  uint16_t ye = y + h - 1;
  for (uint16_t yy = y; yy <= ye; yy++)
  {
    for (uint16_t xx = x; xx <= xe; xx++)
    {
      IO.startTransaction();
      setWindowAddress(xx, yy, xx, yy);
      IO.writeCommand(ILI9488_RAMRD);  // read from RAM
      IO.readData(); // dummy
      uint16_t g = IO.readData();
      uint16_t r = IO.readData();
      uint16_t b = IO.readData();
      *data++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3);
      IO.endTransaction();
    }
  }
}

void GxCTRL_ILI9488_ST::init()
{
  IO.writeCommandTransaction(0x01); // Soft Reset
  delay(150);
  IO.writeCommandTransaction(0x28); // Display Off
  IO.writeCommandTransaction(0x3A); // Pixel read=565, write=565.
  IO.writeDataTransaction(0x55);
  IO.writeCommandTransaction(0xC0); // Power Control 1
  IO.writeDataTransaction(0x10);
  IO.writeDataTransaction(0x10);
  IO.writeCommandTransaction(0xC1); // Power Control 2
  IO.writeDataTransaction(0x41);
  IO.writeCommandTransaction(0xC5); // VCOM  Control 1
  IO.writeDataTransaction(0x00);
  IO.writeDataTransaction(0x22);
  IO.writeDataTransaction(0x80);
  IO.writeDataTransaction(0x40);
  IO.writeCommandTransaction(0x36); // Memory Access
  IO.writeDataTransaction(0x68);
  IO.writeCommandTransaction(0xB0); // Interface
  IO.writeDataTransaction(0x00);
  IO.writeCommandTransaction(0xB1); // Frame Rate Control [B0 11]
  IO.writeDataTransaction(0xB0);
  IO.writeDataTransaction(0x11);
  IO.writeCommandTransaction(0xB4); //Inversion Control [02]
  IO.writeDataTransaction(0x02);
  IO.writeCommandTransaction(0xB6); // Display Function Control [02 02 3B] .kbv NL=480
  IO.writeDataTransaction(0x02);
  IO.writeDataTransaction(0x02);
  IO.writeDataTransaction(0x3B);
  IO.writeCommandTransaction(0xB7); // Entry Mode
  IO.writeDataTransaction(0xC6);
  IO.writeCommandTransaction(0x3A); // Interlace Pixel Format
  IO.writeDataTransaction(0x55);
  IO.writeCommandTransaction(0xF7); // Adjustment Control 3 [A9 51 2C 82]
  IO.writeDataTransaction(0xA9);
  IO.writeDataTransaction(0x51);
  IO.writeDataTransaction(0x2C);
  IO.writeDataTransaction(0x82);
  IO.writeCommandTransaction(0x11); // Sleep Out
  delay(150);
  IO.writeCommandTransaction(0x29); // Display On
}

void GxCTRL_ILI9488_ST::setWindowAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
  // this controller seems to need separate transactions; reason speed on SPI ?
  IO.writeCommandTransaction(ILI9488_CASET);  // Column addr set
  IO.writeDataTransaction(x0 >> 8);
  IO.writeDataTransaction(x0 & 0xFF); // XSTART
  IO.writeDataTransaction(x1 >> 8);
  IO.writeDataTransaction(x1 & 0xFF); // XEND
  IO.writeCommandTransaction(ILI9488_PASET);  // Row addr set
  IO.writeDataTransaction(y0 >> 8);
  IO.writeDataTransaction(y0);        // YSTART
  IO.writeDataTransaction(y1 >> 8);
  IO.writeDataTransaction(y1);        // YEND
  IO.writeCommandTransaction(ILI9488_RAMWR);  // write to RAM
  IO.startTransaction();
}

void GxCTRL_ILI9488_ST::setRotation(uint8_t r)
{
  // this controller seems to need separate transactions; reason speed on SPI ?
  IO.writeCommandTransaction(ILI9488_MADCTL);
  switch (r & 3)
  {
    case 0:
      IO.writeDataTransaction(MADCTL_MX | MADCTL_BGR);
      break;
    case 1:
      IO.writeDataTransaction(MADCTL_MV | MADCTL_BGR);
      break;
    case 2:
      IO.writeDataTransaction(MADCTL_MY | MADCTL_BGR);
      break;
    case 3:
      IO.writeDataTransaction(MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
      break;
  }
}


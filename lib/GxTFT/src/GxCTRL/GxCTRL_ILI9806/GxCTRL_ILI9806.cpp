// created by Jean-Marc Zingg to be the GxCTRL_ILI9806 class for the GxTFT library
// code extracts taken from code and documentation from Ruijia Industry (Tiky_LCD.h, Tiky_LCD.c)
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// note: readRect does not work correctly with my only ILI9806 display (pixel sequence)
//       workaround added

#include "GxCTRL_ILI9806.h"

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

uint32_t GxCTRL_ILI9806::readID()
{
  return readRegister(0xD3, 1, 2);
}

uint32_t GxCTRL_ILI9806::readRegister(uint8_t nr, uint8_t index, uint8_t bytes)
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

uint16_t GxCTRL_ILI9806::readPixel(uint16_t x, uint16_t y)
{
  uint16_t rv;
  readRect(x, y, 1, 1, &rv);
  return rv;
}

#if defined(ILI9806_RAMRD_AUTO_INCREMENT_OK) // not ok on my display

void GxCTRL_ILI9806::readRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t* data)
{
  uint16_t xe = x + w - 1;
  uint16_t ye = y + h - 1;
  uint32_t num = uint32_t(w) * uint32_t(h);
  IO.startTransaction();
  setWindowAddress(x, y, xe, ye);
  IO.writeCommand(RAMRD);  // read from RAM
  IO.readData(); // dummy
  for (; num > 0; num--)
  {
    //*data++ = IO.readData16();
    //*data++ = IO.readData16();
    //*data++ = IO.readData16();
    uint16_t g = IO.readData();
    uint16_t r = IO.readData();
    uint16_t b = IO.readData();
    *data++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3);
  }
  IO.endTransaction();
}

#else

void GxCTRL_ILI9806::readRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t* data)
{
  uint16_t xe = x + w - 1;
  uint16_t ye = y + h - 1;
  for (uint16_t yy = y; yy <= ye; yy++)
  {
    for (uint16_t xx = x; xx <= xe; xx++)
    {
      IO.startTransaction();
      setWindowAddress(xx, yy, xx, yy);
      IO.writeCommand(RAMRD);  // read from RAM
      IO.readData(); // dummy
      uint16_t g = IO.readData();
      uint16_t r = IO.readData();
      uint16_t b = IO.readData();
      *data++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3);
      IO.endTransaction();
    }
  }
}

#endif

void GxCTRL_ILI9806::init()
{
  // Configure ILI9806 display
  //************* Start Initial Sequence **********//
  IO.writeCommandTransaction(0xFF); // EXTC Command Set enable register
  IO.writeDataTransaction(0xFF);
  IO.writeDataTransaction(0x98);
  IO.writeDataTransaction(0x06);

  IO.writeCommandTransaction(0xBA); // SPI Interface Setting
  IO.writeDataTransaction(0xE0);

  IO.writeCommandTransaction(0xBC); // GIP 1
  IO.writeDataTransaction(0x03);
  IO.writeDataTransaction(0x0F);
  IO.writeDataTransaction(0x63);
  IO.writeDataTransaction(0x69);
  IO.writeDataTransaction(0x01);
  IO.writeDataTransaction(0x01);
  IO.writeDataTransaction(0x1B);
  IO.writeDataTransaction(0x11);
  IO.writeDataTransaction(0x70);
  IO.writeDataTransaction(0x73);
  IO.writeDataTransaction(0xFF);
  IO.writeDataTransaction(0xFF);
  IO.writeDataTransaction(0x08);
  IO.writeDataTransaction(0x09);
  IO.writeDataTransaction(0x05);
  IO.writeDataTransaction(0x00);
  IO.writeDataTransaction(0xEE);
  IO.writeDataTransaction(0xE2);
  IO.writeDataTransaction(0x01);
  IO.writeDataTransaction(0x00);
  IO.writeDataTransaction(0xC1);

  IO.writeCommandTransaction(0xBD); // GIP 2
  IO.writeDataTransaction(0x01);
  IO.writeDataTransaction(0x23);
  IO.writeDataTransaction(0x45);
  IO.writeDataTransaction(0x67);
  IO.writeDataTransaction(0x01);
  IO.writeDataTransaction(0x23);
  IO.writeDataTransaction(0x45);
  IO.writeDataTransaction(0x67);

  IO.writeCommandTransaction(0xBE); // GIP 3
  IO.writeDataTransaction(0x00);
  IO.writeDataTransaction(0x22);
  IO.writeDataTransaction(0x27);
  IO.writeDataTransaction(0x6A);
  IO.writeDataTransaction(0xBC);
  IO.writeDataTransaction(0xD8);
  IO.writeDataTransaction(0x92);
  IO.writeDataTransaction(0x22);
  IO.writeDataTransaction(0x22);

  IO.writeCommandTransaction(0xC7); // Vcom
  IO.writeDataTransaction(0x1E);

  IO.writeCommandTransaction(0xED); // EN_volt_reg
  IO.writeDataTransaction(0x7F);
  IO.writeDataTransaction(0x0F);
  IO.writeDataTransaction(0x00);

  IO.writeCommandTransaction(0xC0); // Power Control 1
  IO.writeDataTransaction(0xE3);
  IO.writeDataTransaction(0x0B);
  IO.writeDataTransaction(0x00);

  IO.writeCommandTransaction(0xFC);
  IO.writeDataTransaction(0x08);

  IO.writeCommandTransaction(0xDF); // Engineering Setting
  IO.writeDataTransaction(0x00);
  IO.writeDataTransaction(0x00);
  IO.writeDataTransaction(0x00);
  IO.writeDataTransaction(0x00);
  IO.writeDataTransaction(0x00);
  IO.writeDataTransaction(0x02);

  IO.writeCommandTransaction(0xF3); // DVDD Voltage Setting
  IO.writeDataTransaction(0x74);

  IO.writeCommandTransaction(0xB4); // Display Inversion Control
  IO.writeDataTransaction(0x00);
  IO.writeDataTransaction(0x00);
  IO.writeDataTransaction(0x00);

  IO.writeCommandTransaction(0xF7); // 480x854
  IO.writeDataTransaction(0x81);

  IO.writeCommandTransaction(0xB1); // Frame Rate
  IO.writeDataTransaction(0x00);
  IO.writeDataTransaction(0x10);
  IO.writeDataTransaction(0x14);

  IO.writeCommandTransaction(0xF1); // Panel Timing Control
  IO.writeDataTransaction(0x29);
  IO.writeDataTransaction(0x8A);
  IO.writeDataTransaction(0x07);

  IO.writeCommandTransaction(0xF2); //Panel Timing Control
  IO.writeDataTransaction(0x40);
  IO.writeDataTransaction(0xD2);
  IO.writeDataTransaction(0x50);
  IO.writeDataTransaction(0x28);

  IO.writeCommandTransaction(0xC1); // Power Control 2
  IO.writeDataTransaction(0x17);
  IO.writeDataTransaction(0X85);
  IO.writeDataTransaction(0x85);
  IO.writeDataTransaction(0x20);

  IO.writeCommandTransaction(0xE0);
  IO.writeDataTransaction(0x00); //P1
  IO.writeDataTransaction(0x0C); //P2
  IO.writeDataTransaction(0x15); //P3
  IO.writeDataTransaction(0x0D); //P4
  IO.writeDataTransaction(0x0F); //P5
  IO.writeDataTransaction(0x0C); //P6
  IO.writeDataTransaction(0x07); //P7
  IO.writeDataTransaction(0x05); //P8
  IO.writeDataTransaction(0x07); //P9
  IO.writeDataTransaction(0x0B); //P10
  IO.writeDataTransaction(0x10); //P11
  IO.writeDataTransaction(0x10); //P12
  IO.writeDataTransaction(0x0D); //P13
  IO.writeDataTransaction(0x17); //P14
  IO.writeDataTransaction(0x0F); //P15
  IO.writeDataTransaction(0x00); //P16

  IO.writeCommandTransaction(0xE1);
  IO.writeDataTransaction(0x00); //P1
  IO.writeDataTransaction(0x0D); //P2
  IO.writeDataTransaction(0x15); //P3
  IO.writeDataTransaction(0x0E); //P4
  IO.writeDataTransaction(0x10); //P5
  IO.writeDataTransaction(0x0D); //P6
  IO.writeDataTransaction(0x08); //P7
  IO.writeDataTransaction(0x06); //P8
  IO.writeDataTransaction(0x07); //P9
  IO.writeDataTransaction(0x0C); //P10
  IO.writeDataTransaction(0x11); //P11
  IO.writeDataTransaction(0x11); //P12
  IO.writeDataTransaction(0x0E); //P13
  IO.writeDataTransaction(0x17); //P14
  IO.writeDataTransaction(0x0F); //P15
  IO.writeDataTransaction(0x00); //P16

  IO.writeCommandTransaction(0x35); //Tearing Effect ON
  IO.writeDataTransaction(0x00);

  IO.writeCommandTransaction(0x36);
  IO.writeDataTransaction(0x60);

  IO.writeCommandTransaction(0x3A);
  IO.writeDataTransaction(0x55);

  IO.writeCommandTransaction(0x11); //Exit Sleep
  delay(120);
  IO.writeCommandTransaction(0x29); // Display On
  delay(10);

  IO.writeCommandTransaction(0x3A);
  IO.writeDataTransaction(0x55);
  IO.writeCommandTransaction(0x36);
  IO.writeDataTransaction(0x0); // portrait, RGB
}

void GxCTRL_ILI9806::setWindowAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
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

void GxCTRL_ILI9806::setRotation(uint8_t r)
{
  IO.startTransaction();
  IO.writeCommand(MADCTL);
  switch (r & 3)
  {
    case 0:
      IO.writeData(0);
      break;
    case 1:
      IO.writeData(MADCTL_MX | MADCTL_MV);
      break;
    case 2:
      IO.writeData(MADCTL_MX | MADCTL_MY);
      break;
    case 3:
      IO.writeData(MADCTL_MY | MADCTL_MV);
      break;
  }
  IO.endTransaction();
}


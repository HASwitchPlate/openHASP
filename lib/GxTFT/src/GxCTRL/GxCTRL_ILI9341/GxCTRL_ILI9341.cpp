// created by Jean-Marc Zingg to be the GxCTRL_ILI9341 class for the GxTFT library
// code extracts taken from https://github.com/Bodmer/TFT_HX8357
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// note: readRect does not work correctly with my only ILI9341 display (pixel sequence)
//       workaround added

#include "GxCTRL_ILI9341.h"

// These are the ILI9341 control registers
#define ILI9341_NOP     0x00
#define ILI9341_SWRESET 0x01
#define ILI9341_RDDID   0x04
#define ILI9341_RDDST   0x09

#define ILI9341_SLPIN   0x10
#define ILI9341_SLPOUT  0x11
#define ILI9341_PTLON   0x12
#define ILI9341_NORON   0x13

#define ILI9341_RDMODE  0x0A
#define ILI9341_RDMADCTL  0x0B
#define ILI9341_RDPIXFMT  0x0C
#define ILI9341_RDIMGFMT  0x0A
#define ILI9341_RDSELFDIAG  0x0F

#define ILI9341_INVOFF  0x20
#define ILI9341_INVON   0x21
#define ILI9341_GAMMASET 0x26
#define ILI9341_DISPOFF 0x28
#define ILI9341_DISPON  0x29

#define ILI9341_CASET   0x2A
#define ILI9341_PASET   0x2B
#define ILI9341_RAMWR   0x2C
#define ILI9341_RAMRD   0x2E

#define ILI9341_PTLAR   0x30
#define ILI9341_VSCRDEF 0x33
#define ILI9341_MADCTL  0x36
#define ILI9341_VSCRSADD 0x37
#define ILI9341_PIXFMT  0x3A

#define ILI9341_FRMCTR1 0xB1
#define ILI9341_FRMCTR2 0xB2
#define ILI9341_FRMCTR3 0xB3
#define ILI9341_INVCTR  0xB4
#define ILI9341_DFUNCTR 0xB6

#define ILI9341_PWCTR1  0xC0
#define ILI9341_PWCTR2  0xC1
#define ILI9341_PWCTR3  0xC2
#define ILI9341_PWCTR4  0xC3
#define ILI9341_PWCTR5  0xC4
#define ILI9341_VMCTR1  0xC5
#define ILI9341_VMCTR2  0xC7

#define ILI9341_RDID1   0xDA
#define ILI9341_RDID2   0xDB
#define ILI9341_RDID3   0xDC
#define ILI9341_RDID4   0xDD

#define ILI9341_GMCTRP1 0xE0
#define ILI9341_GMCTRN1 0xE1

#define ILI9341_MADCTL_MY  0x80
#define ILI9341_MADCTL_MX  0x40
#define ILI9341_MADCTL_MV  0x20
#define ILI9341_MADCTL_ML  0x10
#define ILI9341_MADCTL_RGB 0x00
#define ILI9341_MADCTL_BGR 0x08
#define ILI9341_MADCTL_MH  0x04

uint32_t GxCTRL_ILI9341::readID()
{
  return readRegister(0xD3, 0, 3);
}

uint32_t GxCTRL_ILI9341::readRegister(uint8_t nr, uint8_t index, uint8_t bytes)
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

uint16_t GxCTRL_ILI9341::readPixel(uint16_t x, uint16_t y)
{
  uint16_t rv;
  readRect(x, y, 1, 1, &rv);
  return rv;
}

#if defined(ILI9341_RAMRD_AUTO_INCREMENT_OK) // not ok on my display

void GxCTRL_ILI9341::readRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t* data)
{
  uint16_t xe = x + w - 1;
  uint16_t ye = y + h - 1;
  uint32_t num = uint32_t(w) * uint32_t(h);
  IO.startTransaction();
  IO.writeCommand(ILI9341_CASET);  // Column addr set
  IO.writeData(x >> 8);
  IO.writeData(x & 0xFF);  // XSTART
  IO.writeData(xe >> 8);
  IO.writeData(xe & 0xFF); // XEND
  IO.writeCommand(ILI9341_PASET);  // Row addr set
  IO.writeData(y >> 8);
  IO.writeData(y);         // YSTART
  IO.writeData(ye >> 8);
  IO.writeData(ye);        // YEND
  IO.writeCommand(ILI9341_RAMRD);  // read from RAM
  IO.readData(); // dummy
  for (; num > 0; num--)
  {
    uint16_t g = IO.readData();
    uint16_t r = IO.readData();
    uint16_t b = IO.readData();
    *data++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3);
  }
  IO.endTransaction();
}

#else

void GxCTRL_ILI9341::readRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t* data)
{
  uint16_t xe = x + w - 1;
  uint16_t ye = y + h - 1;
  for (uint16_t yy = y; yy <= ye; yy++)
  {
    for (uint16_t xx = x; xx <= xe; xx++)
    {
      IO.startTransaction();
      setWindowAddress(xx, yy, xx, yy);
      IO.writeCommand(ILI9341_RAMRD);  // read from RAM
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

void GxCTRL_ILI9341::init()
{
  IO.startTransaction();
  IO.writeCommand(0xEF);
  IO.writeData(0x03);
  IO.writeData(0x80);
  IO.writeData(0x02);

  IO.writeCommand(0xCF);
  IO.writeData(0x00);
  IO.writeData(0XC1);
  IO.writeData(0X30);

  IO.writeCommand(0xED);
  IO.writeData(0x64);
  IO.writeData(0x03);
  IO.writeData(0X12);
  IO.writeData(0X81);

  IO.writeCommand(0xE8);
  IO.writeData(0x85);
  IO.writeData(0x00);
  IO.writeData(0x78);

  IO.writeCommand(0xCB);
  IO.writeData(0x39);
  IO.writeData(0x2C);
  IO.writeData(0x00);
  IO.writeData(0x34);
  IO.writeData(0x02);

  IO.writeCommand(0xF7);
  IO.writeData(0x20);

  IO.writeCommand(0xEA);
  IO.writeData(0x00);
  IO.writeData(0x00);

  IO.writeCommand(ILI9341_PWCTR1);    //Power control
  IO.writeData(0x23);   //VRH[5:0]

  IO.writeCommand(ILI9341_PWCTR2);    //Power control
  IO.writeData(0x10);   //SAP[2:0];BT[3:0]

  IO.writeCommand(ILI9341_VMCTR1);    //VCM control
  IO.writeData(0x3e);
  IO.writeData(0x28);

  IO.writeCommand(ILI9341_VMCTR2);    //VCM control2
  IO.writeData(0x86);  //--

  IO.writeCommand(ILI9341_MADCTL);    // Memory Access Control
  IO.writeData(0x48);

  IO.writeCommand(ILI9341_PIXFMT);
  IO.writeData(0x55);

  IO.writeCommand(ILI9341_FRMCTR1);
  IO.writeData(0x00);
  IO.writeData(0x18);

  IO.writeCommand(ILI9341_DFUNCTR);    // Display Function Control
  IO.writeData(0x08);
  IO.writeData(0x82);
  IO.writeData(0x27);

  IO.writeCommand(0xF2);    // 3Gamma Function Disable
  IO.writeData(0x00);

  IO.writeCommand(ILI9341_GAMMASET);    //Gamma curve selected
  IO.writeData(0x01);

  IO.writeCommand(ILI9341_GMCTRP1);    //Set Gamma
  IO.writeData(0x0F);
  IO.writeData(0x31);
  IO.writeData(0x2B);
  IO.writeData(0x0C);
  IO.writeData(0x0E);
  IO.writeData(0x08);
  IO.writeData(0x4E);
  IO.writeData(0xF1);
  IO.writeData(0x37);
  IO.writeData(0x07);
  IO.writeData(0x10);
  IO.writeData(0x03);
  IO.writeData(0x0E);
  IO.writeData(0x09);
  IO.writeData(0x00);

  IO.writeCommand(ILI9341_GMCTRN1);    //Set Gamma
  IO.writeData(0x00);
  IO.writeData(0x0E);
  IO.writeData(0x14);
  IO.writeData(0x03);
  IO.writeData(0x11);
  IO.writeData(0x07);
  IO.writeData(0x31);
  IO.writeData(0xC1);
  IO.writeData(0x48);
  IO.writeData(0x08);
  IO.writeData(0x0F);
  IO.writeData(0x0C);
  IO.writeData(0x31);
  IO.writeData(0x36);
  IO.writeData(0x0F);

  IO.writeCommand(ILI9341_SLPOUT);    //Exit Sleep
  IO.endTransaction();
  delay(120);
  IO.startTransaction();
  IO.writeCommand(ILI9341_DISPON);    //Display on
  IO.endTransaction();
}

void GxCTRL_ILI9341::setWindowAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
  IO.writeCommand(ILI9341_CASET);  // Column addr set
  IO.writeData(x0 >> 8);
  IO.writeData(x0 & 0xFF); // XSTART
  IO.writeData(x1 >> 8);
  IO.writeData(x1 & 0xFF); // XEND
  IO.writeCommand(ILI9341_PASET);  // Row addr set
  IO.writeData(y0 >> 8);
  IO.writeData(y0);        // YSTART
  IO.writeData(y1 >> 8);
  IO.writeData(y1);        // YEND
  IO.writeCommand(ILI9341_RAMWR);  // write to RAM
}

void GxCTRL_ILI9341::setRotation(uint8_t r)
{
  IO.startTransaction();
  IO.writeCommand(ILI9341_MADCTL);
  switch (r & 3)
  {
    case 0: // Portrait
      IO.writeData(ILI9341_MADCTL_BGR | ILI9341_MADCTL_MX);
      break;
    case 1: // Landscape (Portrait + 90)
      IO.writeData(ILI9341_MADCTL_BGR | ILI9341_MADCTL_MV);
      break;
    case 2: // Inverter portrait
      IO.writeData(ILI9341_MADCTL_BGR | ILI9341_MADCTL_MY);
      break;
    case 3: // Inverted landscape
      IO.writeData(ILI9341_MADCTL_BGR | ILI9341_MADCTL_MV | ILI9341_MADCTL_MX | ILI9341_MADCTL_MY);
      break;
  }
  IO.endTransaction();
}


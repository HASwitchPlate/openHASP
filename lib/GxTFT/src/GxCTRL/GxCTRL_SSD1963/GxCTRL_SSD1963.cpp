// created by Jean-Marc Zingg to be the GxCTRL_SSD1963 class for the GxTFT library
// code extracts taken from http://www.rinkydinkelectronics.com/download.php?f=UTFT.zip
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE

#include "GxCTRL_SSD1963.h"

uint32_t GxCTRL_SSD1963::readID()
{
  return readRegister(0xA1, 0, 4);
  uint32_t rv = 0;
  IO.startTransaction();
  IO.writeCommand(0xA1);
  rv |= IO.readData16();
  rv |= uint32_t(IO.readData16()) << 16;
  IO.endTransaction();
  return rv;
}

uint32_t GxCTRL_SSD1963::readRegister(uint8_t nr, uint8_t index, uint8_t bytes)
{
  uint32_t rv = 0;
  bytes = min(bytes, 4);
  // only "get" commands allowed!
  switch (nr)
  {
    case 0x0A: // Get Power Mode
    case 0x0B: // Get Address Mode
    case 0x0C: // Get Pixel Format
    case 0x0D: // Get Display Mode
    case 0x0E: // Get Signal Mode
      IO.startTransaction();
      IO.writeCommand(nr);
      rv |= IO.readData();
      IO.endTransaction();
      break;
    case 0x45: // Get Tear Scanline
      IO.startTransaction();
      IO.writeCommand(nr);
      rv |= IO.readData() << 8;
      rv |= IO.readData();
      IO.endTransaction();
      break;
    case 0xA1: // Get DDB
    case 0xB1: // Get LCD Mode
    case 0xB5: // Get Horizontal Period
    case 0xB7: // Get Vertical Period
    case 0xB9: // Get GPIO Configuration
    case 0xBB: // Get GPIO Value
    case 0xBD: // Get Post Proc
    case 0xBF: // Get PWM Configuration
    case 0xC1: // Get LCD Gen0
    case 0xC3: // Get LCD Gen1
    case 0xC5: // Get LCD Gen2
    case 0xC7: // Get LCD Gen3
    case 0xC9: // Get GPIO0 ROP
    case 0xCB: // Get GPIO1 ROP
    case 0xCD: // Get GPIO2 ROP
    case 0xCF: // Get GPIO3 ROP
    case 0xD1: // Get DBC Configuration
    case 0xD5: // Get DBC Threshold
    case 0xE3: // Get PLL MN
    case 0xE4: // Get PLL Status
    case 0xE7: // Get LSHIFT Frequency
    case 0xF1: // Get Pixel Data Interface
      IO.startTransaction();
      IO.writeCommand(nr);
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
      break;
  }
  return rv;
}

uint16_t GxCTRL_SSD1963::readPixel(uint16_t x, uint16_t y)
{
  uint16_t rv;
  readRect(x, y, 1, 1, &rv);
  return rv;
}

#if defined(SSD1963_RAMRD_AUTO_INCREMENT_OK) // not ok on my display

void GxCTRL_SSD1963::readRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t* data)
{
  uint16_t xe = x + w - 1;
  uint16_t ye = y + h - 1;
  uint32_t num = uint32_t(w) * uint32_t(h);
  IO.startTransaction();
  setWindowAddress(x, y, xe, ye);
  IO.writeCommand(0x2e);  // read from RAM
  //IO.readData(); // dummy
  for (; num > 0; num--)
  {
    uint16_t d = IO.readData16();
    *data++ = ((d & 0x001F) << 11) | (d & 0x07E0) | ((d & 0xF800) >> 11); // r,b swapped
  }
  IO.endTransaction();
}

#else

void GxCTRL_SSD1963::readRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t* data)
{
  uint16_t xe = x + w - 1;
  uint16_t ye = y + h - 1;
  uint32_t num = uint32_t(w) * uint32_t(h);
  for (uint16_t yy = y; yy <= ye; yy++)
  {
    for (uint16_t xx = x; xx <= xe; xx++)
    {
      IO.startTransaction();
      setWindowAddress(xx, yy, xe, ye);
      IO.writeCommand(0x2e);
      *data++ = IO.readData16();
      IO.endTransaction();
    }
  }
}

#endif

void GxCTRL_SSD1963::init()
{
  rotation = 1; // landscape is default
  IO.startTransaction();
  IO.writeCommand(0xE2);    //PLL multiplier, set PLL clock to 120M
  IO.writeData(0x23);     //N=0x36 for 6.5M, 0x23 for 10M crystal
  IO.writeData(0x02);
  IO.writeData(0x04); // original SSD1963_800ALT
  IO.writeCommand(0xE0);    // PLL enable
  IO.writeData(0x01);
  delay(10);
  IO.writeCommand(0xE0);
  IO.writeData(0x03);
  delay(10);
  IO.writeCommand(0x01);    // software reset
  delay(100);
  IO.writeCommand(0xE6);    //PLL setting for PCLK, depends on resolution
  IO.writeData(0x04);
  IO.writeData(0x93);
  IO.writeData(0xE0);

  IO.writeCommand(0xB0);    //LCD SPECIFICATION
  IO.writeData(0x00); // 0x24
  IO.writeData(0x00);
  IO.writeAddrMSBfirst(physical_width - 1); // Set HDP
  //IO.writeData(0x03);   //Set HDP 799
  //IO.writeData(0x1F);
  IO.writeAddrMSBfirst(physical_height - 1); // Set VDP
  //IO.writeData(0x01);   //Set VDP 479
  //IO.writeData(0xDF);
  IO.writeData(0x00);

  IO.writeCommand(0xB4);    //HSYNC
  IO.writeData(0x03);   //Set HT  928
  IO.writeData(0xA0);
  IO.writeData(0x00);   //Set HPS 46
  IO.writeData(0x2E);
  IO.writeData(0x30);   //Set HPW 48
  IO.writeData(0x00);   //Set LPS 15
  IO.writeData(0x0F);
  IO.writeData(0x00);

  IO.writeCommand(0xB6);    //VSYNC
  IO.writeData(0x02);   //Set VT  525
  IO.writeData(0x0D);
  IO.writeData(0x00);   //Set VPS 16
  IO.writeData(0x10);
  IO.writeData(0x10);   //Set VPW 16
  IO.writeData(0x00);   //Set FPS 8
  IO.writeData(0x08);

  IO.writeCommand(0xBA);
  //IO.writeData(0x05);   //GPIO[3:0] out 1
  IO.writeData(0x07);   //GPIO[3:0] out 1

  IO.writeCommand(0xB8);
  IO.writeData(0x07);     //GPIO3=input, GPIO[2:0]=output
  IO.writeData(0x01);   //GPIO0 normal

  IO.writeCommand(0x36);    //rotation
  IO.writeData(0x22);   // -- Set to 0x21 to rotate 180 degrees

  IO.writeCommand(0xF0);    //pixel data interface
  IO.writeData(0x03);
  IO.endTransaction();

  delay(10);

  IO.startTransaction();
  // SetWindow, physical addresses, even if default rotation is changed
  IO.writeCommand(0x2a);
  IO.writeAddrMSBfirst(0);
  IO.writeAddrMSBfirst(physical_width - 1);
  IO.writeCommand(0x2b);
  IO.writeAddrMSBfirst(0);
  IO.writeAddrMSBfirst(physical_height - 1);

  IO.writeCommand(0x29);    //display on

  IO.writeCommand(0xBE);    //set PWM for B/L
  IO.writeData(0x06);
  IO.writeData(0xF0);
  IO.writeData(0x01);
  IO.writeData(0xF0);
  IO.writeData(0x00);
  IO.writeData(0x00);

  IO.writeCommand(0xD0);
  IO.writeData(0x0D);

  IO.writeCommand(0x2C);
  IO.endTransaction();
}

void GxCTRL_SSD1963::setWindowAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
  if (rotation & 1) // landscape
  {
    IO.writeCommand(0x2a);
    IO.writeAddrMSBfirst(x0);
    IO.writeAddrMSBfirst(x1);
    IO.writeCommand(0x2b);
    IO.writeAddrMSBfirst(y0);
    IO.writeAddrMSBfirst(y1);
    IO.writeCommand(0x2c);
  }
  else // portrait
  {
    // transform to physical addresses
    IO.writeCommand(0x2b); // by switching address
    IO.writeAddrMSBfirst(x0);
    IO.writeAddrMSBfirst(x1);
    IO.writeCommand(0x2a); // by switching address
    IO.writeAddrMSBfirst(y0);
    IO.writeAddrMSBfirst(y1);
    IO.writeCommand(0x2c);
  }
  IO.writeCommand(0x2c);
}

void GxCTRL_SSD1963::setRotation(uint8_t r)
{
  rotation = r;
  IO.startTransaction();
  IO.writeCommand(0x36);
  switch (r & 3)
  {
    case 0:
      IO.writeData(0x33);
      break;
    case 1:
      IO.writeData(0x2);
      break;
    case 2:
      IO.writeData(0x20);
      break;
    case 3:
      IO.writeData(0x11);
      break;
  }
  IO.endTransaction();
}


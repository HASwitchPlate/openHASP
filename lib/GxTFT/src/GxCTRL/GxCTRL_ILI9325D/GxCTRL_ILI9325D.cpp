// created by Jean-Marc Zingg to be the GxCTRL_ILI9325D class for the GxTFT library
// code extracts taken from http://www.rinkydinkelectronics.com/download.php?f=UTFT.zip
// code extracts taken from https://github.com/adafruit/TFTLCD-Library
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE

#include "GxCTRL_ILI9325D.h"

uint32_t GxCTRL_ILI9325D::readID()
{
  return readRegister(0x0, 0, 2);
}

uint32_t GxCTRL_ILI9325D::readRegister(uint8_t nr, uint8_t index, uint8_t bytes)
{
  uint32_t rv = 0;
  bytes = min(bytes, 4);
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
  return rv;
}

uint16_t GxCTRL_ILI9325D::readPixel(uint16_t x, uint16_t y)
{
  uint16_t rv;
  readRect(x, y, 1, 1, &rv);
  return rv;
}

void GxCTRL_ILI9325D::readRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t* data)
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
      IO.writeCommand(0x22);
      IO.readData16(); // dummy
      *data++ = IO.readData16();
      IO.endTransaction();
    }
  }
}

void GxCTRL_ILI9325D::init()
{
  _rotation = 0; // portrait is default
  IO.startTransaction();
  LCD_Write_COM_DATA(0xE5, 0x78F0); // set SRAM internal timing
  LCD_Write_COM_DATA(0x01, 0x0100); // set Driver Output Control
  LCD_Write_COM_DATA(0x02, 0x0200); // set 1 line inversion
  LCD_Write_COM_DATA(0x03, 0x1030); // set GRAM write direction and BGR=1.
  LCD_Write_COM_DATA(0x04, 0x0000); // Resize register
  LCD_Write_COM_DATA(0x08, 0x0207); // set the back porch and front porch
  LCD_Write_COM_DATA(0x09, 0x0000); // set non-display area refresh cycle ISC[3:0]
  LCD_Write_COM_DATA(0x0A, 0x0000); // FMARK function
  LCD_Write_COM_DATA(0x0C, 0x0000); // RGB interface setting
  LCD_Write_COM_DATA(0x0D, 0x0000); // Frame marker Position
  LCD_Write_COM_DATA(0x0F, 0x0000); // RGB interface polarity
  //*************Power On sequence ****************//
  LCD_Write_COM_DATA(0x10, 0x0000); // SAP, BT[3:0], AP, DSTB, SLP, STB
  LCD_Write_COM_DATA(0x11, 0x0007); // DC1[2:0], DC0[2:0], VC[2:0]
  LCD_Write_COM_DATA(0x12, 0x0000); // VREG1OUT voltage
  LCD_Write_COM_DATA(0x13, 0x0000); // VDV[4:0] for VCOM amplitude
  LCD_Write_COM_DATA(0x07, 0x0001);
  delay(200); // Dis-charge capacitor power voltage
  LCD_Write_COM_DATA(0x10, 0x1690); // SAP, BT[3:0], AP, DSTB, SLP, STB
  LCD_Write_COM_DATA(0x11, 0x0227); // Set DC1[2:0], DC0[2:0], VC[2:0]
  delay(50); // Delay 50ms
  LCD_Write_COM_DATA(0x12, 0x000D); // 0012
  delay(50); // Delay 50ms
  LCD_Write_COM_DATA(0x13, 0x1200); // VDV[4:0] for VCOM amplitude
  LCD_Write_COM_DATA(0x29, 0x000A); // 04  VCM[5:0] for VCOMH
  LCD_Write_COM_DATA(0x2B, 0x000D); // Set Frame Rate
  delay(50); // Delay 50ms
  LCD_Write_COM_DATA(0x20, 0x0000); // GRAM horizontal Address
  LCD_Write_COM_DATA(0x21, 0x0000); // GRAM Vertical Address
  // ----------- Adjust the Gamma Curve ----------//
  LCD_Write_COM_DATA(0x30, 0x0000);
  LCD_Write_COM_DATA(0x31, 0x0404);
  LCD_Write_COM_DATA(0x32, 0x0003);
  LCD_Write_COM_DATA(0x35, 0x0405);
  LCD_Write_COM_DATA(0x36, 0x0808);
  LCD_Write_COM_DATA(0x37, 0x0407);
  LCD_Write_COM_DATA(0x38, 0x0303);
  LCD_Write_COM_DATA(0x39, 0x0707);
  LCD_Write_COM_DATA(0x3C, 0x0504);
  LCD_Write_COM_DATA(0x3D, 0x0808);
  //------------------ Set GRAM area ---------------//
  LCD_Write_COM_DATA(0x50, 0x0000); // Horizontal GRAM Start Address
  LCD_Write_COM_DATA(0x51, 0x00EF); // Horizontal GRAM End Address
  LCD_Write_COM_DATA(0x52, 0x0000); // Vertical GRAM Start Address
  LCD_Write_COM_DATA(0x53, 0x013F); // Vertical GRAM Start Address
  LCD_Write_COM_DATA(0x60, 0xA700); // Gate Scan Line
  LCD_Write_COM_DATA(0x61, 0x0001); // NDL,VLE, REV
  LCD_Write_COM_DATA(0x6A, 0x0000); // set scrolling line
  //-------------- Partial Display Control ---------//
  LCD_Write_COM_DATA(0x80, 0x0000);
  LCD_Write_COM_DATA(0x81, 0x0000);
  LCD_Write_COM_DATA(0x82, 0x0000);
  LCD_Write_COM_DATA(0x83, 0x0000);
  LCD_Write_COM_DATA(0x84, 0x0000);
  LCD_Write_COM_DATA(0x85, 0x0000);
  //-------------- Panel Control -------------------//
  LCD_Write_COM_DATA(0x90, 0x0010);
  LCD_Write_COM_DATA(0x92, 0x0000);
  LCD_Write_COM_DATA(0x07, 0x0133); // 262K color and display ON
  IO.endTransaction();
}

void GxCTRL_ILI9325D::setWindowAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
  switch (_rotation)
  {
    case 0:
      LCD_Write_COM_DATA(0x50, x0); // horizontal address start
      LCD_Write_COM_DATA(0x51, x1); // horizontal address end
      LCD_Write_COM_DATA(0x52, y0); // vertical address start
      LCD_Write_COM_DATA(0x53, y1); // vertical address end
      LCD_Write_COM_DATA(0x20, x0); // horizontal GRAM address set
      LCD_Write_COM_DATA(0x21, y0); // vertical GRAM address set
      break;
    case 1:
      LCD_Write_COM_DATA(0x50, _tft_width - 1 - y1); // horizontal address start
      LCD_Write_COM_DATA(0x51, _tft_width - 1 - y0); // horizontal address end
      LCD_Write_COM_DATA(0x52, x0); // vertical address start
      LCD_Write_COM_DATA(0x53, x1); // vertical address end
      LCD_Write_COM_DATA(0x20, _tft_width - 1 - y0); // horizontal GRAM address set
      LCD_Write_COM_DATA(0x21, x0); // vertical GRAM address set
      break;
    case 2:
      LCD_Write_COM_DATA(0x50, _tft_width - 1 - x1); // horizontal address start
      LCD_Write_COM_DATA(0x51, _tft_width - 1 - x0); // horizontal address end
      LCD_Write_COM_DATA(0x52, _tft_height - 1 - y1); // vertical address start
      LCD_Write_COM_DATA(0x53, _tft_height - 1 - y0); // vertical address end
      LCD_Write_COM_DATA(0x20, _tft_width - 1 - x0); // horizontal GRAM address set
      LCD_Write_COM_DATA(0x21, _tft_height - 1 - y0); // vertical GRAM address set
      break;
    case 3:
      LCD_Write_COM_DATA(0x50, y0); // horizontal address start
      LCD_Write_COM_DATA(0x51, y1); // horizontal address end
      LCD_Write_COM_DATA(0x52, _tft_height - 1 - x1); // vertical address start
      LCD_Write_COM_DATA(0x53, _tft_height - 1 - x0); // vertical address end
      LCD_Write_COM_DATA(0x20, y0); // horizontal GRAM address set
      LCD_Write_COM_DATA(0x21, _tft_height - 1 - x0); // vertical GRAM address set
      break;
  }
  IO.writeCommand(0x22);
}

void GxCTRL_ILI9325D::setRotation(uint8_t r)
{
  _rotation = r & 3;
  IO.startTransaction();
  switch (_rotation)
  {
    case 0: // Portrait
      LCD_Write_COM_DATA(0x03, 0x1030);
      break;
    case 1: // Landscape (Portrait + 90)
      LCD_Write_COM_DATA(0x03, 0x1028);
      break;
    case 2: // Inverter portrait
      LCD_Write_COM_DATA(0x03, 0x1080);
      break;
    case 3: // Inverted landscape
      LCD_Write_COM_DATA(0x03, 0x1018);
      break;
  }
  IO.endTransaction();
}

void GxCTRL_ILI9325D::LCD_Write_COM_DATA(uint8_t com, uint16_t data)
{
  IO.writeCommand(com);
  IO.writeData(data >> 8);
  IO.writeData(data);
}


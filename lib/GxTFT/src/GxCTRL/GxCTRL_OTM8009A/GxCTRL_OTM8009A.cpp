// created by Jean-Marc Zingg to be the GxCTRL_OTM8009A class for the GxTFT library
// code extracts taken from code and documentation from Ruijia Industry (lcd.h, lcd.c)
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// this class works with "IPS 3.97 inch 16.7M HD TFT LCD Touch Screen Module OTM8009A Drive IC 480(RGB)*800" display from Ruijia Industry
// e.g. https://www.aliexpress.com/item/IPS-3-97-inch-HD-TFT-LCD-Touch-Screen-Module-OTM8009A-Drive-IC-800-480/32676929794.html
// this display matches the FSMC TFT connector of the STM32F407ZG-M4 board, EXCEPT FOR POWER SUPPLY PINS
// e.g. https://www.aliexpress.com/item/STM32F407ZGT6-Development-Board-ARM-M4-STM32F4-cortex-M4-core-Board-Compatibility-Multiple-Extension/32795142050.html
// CAUTION: the display needs 5V on VCC pins; data and control pins are 3.3V
//
// note: this display needs 16bit commands, aka "(MDDI/SPI) Address" in some OTM8009A.pdf

#include "GxCTRL_OTM8009A.h"

#define OTM8009A_MADCTL     0x3600
#define OTM8009A_MADCTL_MY  0x80
#define OTM8009A_MADCTL_MX  0x40
#define OTM8009A_MADCTL_MV  0x20
#define OTM8009A_MADCTL_ML  0x10
#define OTM8009A_MADCTL_RGB 0x00
#define OTM8009A_MADCTL_BGR 0x08

uint32_t GxCTRL_OTM8009A::readID()
{
  return readRegister(0xA1, 2, 2);
}

uint32_t GxCTRL_OTM8009A::readRegister(uint8_t nr, uint8_t index, uint8_t bytes)
{
  uint32_t rv = 0;
  bytes = gx_uint8_min(bytes, 4);
  IO.startTransaction();
  IO.writeCommand16(nr << 8);
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

uint16_t GxCTRL_OTM8009A::readPixel(uint16_t x, uint16_t y)
{
  uint16_t rv;
  readRect(x, y, 1, 1, &rv);
  return rv;
}

#if defined(OTM8009A_RAMRD_AUTO_INCREMENT_OK) // not ok on my display

void GxCTRL_OTM8009A::readRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t* data)
{
  uint16_t xe = x + w - 1;
  uint16_t ye = y + h - 1;
  uint32_t num = uint32_t(w) * uint32_t(h);
  IO.startTransaction();
  setWindowAddress(x, y, xe, ye);
  IO.writeCommand16(0x2E00);  // read from RAM
  IO.readData16(); // dummy
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

void GxCTRL_OTM8009A::readRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t* data)
{
  uint16_t xe = x + w - 1;
  uint16_t ye = y + h - 1;
  for (uint16_t yy = y; yy <= ye; yy++)
  {
    for (uint16_t xx = x; xx <= xe; xx++)
    {
      IO.startTransaction();
      setWindowAddress(xx, yy, xx, yy);
      IO.writeCommand16(0x2E00);  // read from RAM
      IO.readData16(); // dummy
      uint16_t g = IO.readData();
      uint16_t r = IO.readData();
      uint16_t b = IO.readData();
      *data++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3);
      IO.endTransaction();
    }
  }
}

#endif

void GxCTRL_OTM8009A::init()
{
  //============ OTM8009A+HSD3.97 20140613 ===============================================//
  IO.writeCommand16Transaction(0xff00);      IO.writeData16Transaction(0x80);    //enable access command2
  IO.writeCommand16Transaction(0xff01);      IO.writeData16Transaction(0x09);    //enable access command2
  IO.writeCommand16Transaction(0xff02);      IO.writeData16Transaction(0x01);    //enable access command2
  IO.writeCommand16Transaction(0xff80);      IO.writeData16Transaction(0x80);    //enable access command2
  IO.writeCommand16Transaction(0xff81);      IO.writeData16Transaction(0x09);    //enable access command2
  IO.writeCommand16Transaction(0xff03);      IO.writeData16Transaction(0x01);    //
  IO.writeCommand16Transaction(0xc5b1);      IO.writeData16Transaction(0xA9);    //power control

  IO.writeCommand16Transaction(0xc591);      IO.writeData16Transaction(0x0F);               //power control
  IO.writeCommand16Transaction(0xc0B4);      IO.writeData16Transaction(0x50);

  //panel driving mode : column inversion

  //////  gamma
  IO.writeCommand16Transaction(0xE100);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xE101);      IO.writeData16Transaction(0x09);
  IO.writeCommand16Transaction(0xE102);      IO.writeData16Transaction(0x0F);
  IO.writeCommand16Transaction(0xE103);      IO.writeData16Transaction(0x0E);
  IO.writeCommand16Transaction(0xE104);      IO.writeData16Transaction(0x07);
  IO.writeCommand16Transaction(0xE105);      IO.writeData16Transaction(0x10);
  IO.writeCommand16Transaction(0xE106);      IO.writeData16Transaction(0x0B);
  IO.writeCommand16Transaction(0xE107);      IO.writeData16Transaction(0x0A);
  IO.writeCommand16Transaction(0xE108);      IO.writeData16Transaction(0x04);
  IO.writeCommand16Transaction(0xE109);      IO.writeData16Transaction(0x07);
  IO.writeCommand16Transaction(0xE10A);      IO.writeData16Transaction(0x0B);
  IO.writeCommand16Transaction(0xE10B);      IO.writeData16Transaction(0x08);
  IO.writeCommand16Transaction(0xE10C);      IO.writeData16Transaction(0x0F);
  IO.writeCommand16Transaction(0xE10D);      IO.writeData16Transaction(0x10);
  IO.writeCommand16Transaction(0xE10E);      IO.writeData16Transaction(0x0A);
  IO.writeCommand16Transaction(0xE10F);      IO.writeData16Transaction(0x01);
  IO.writeCommand16Transaction(0xE200);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xE201);      IO.writeData16Transaction(0x09);
  IO.writeCommand16Transaction(0xE202);      IO.writeData16Transaction(0x0F);
  IO.writeCommand16Transaction(0xE203);      IO.writeData16Transaction(0x0E);
  IO.writeCommand16Transaction(0xE204);      IO.writeData16Transaction(0x07);
  IO.writeCommand16Transaction(0xE205);      IO.writeData16Transaction(0x10);
  IO.writeCommand16Transaction(0xE206);      IO.writeData16Transaction(0x0B);
  IO.writeCommand16Transaction(0xE207);      IO.writeData16Transaction(0x0A);
  IO.writeCommand16Transaction(0xE208);      IO.writeData16Transaction(0x04);
  IO.writeCommand16Transaction(0xE209);      IO.writeData16Transaction(0x07);
  IO.writeCommand16Transaction(0xE20A);      IO.writeData16Transaction(0x0B);
  IO.writeCommand16Transaction(0xE20B);      IO.writeData16Transaction(0x08);
  IO.writeCommand16Transaction(0xE20C);      IO.writeData16Transaction(0x0F);
  IO.writeCommand16Transaction(0xE20D);      IO.writeData16Transaction(0x10);
  IO.writeCommand16Transaction(0xE20E);      IO.writeData16Transaction(0x0A);
  IO.writeCommand16Transaction(0xE20F);      IO.writeData16Transaction(0x01);
  IO.writeCommand16Transaction(0xD900);      IO.writeData16Transaction(0x4E);    //vcom setting

  IO.writeCommand16Transaction(0xc181);      IO.writeData16Transaction(0x66);    //osc=65HZ

  IO.writeCommand16Transaction(0xc1a1);      IO.writeData16Transaction(0x08);
  IO.writeCommand16Transaction(0xc592);      IO.writeData16Transaction(0x01);    //power control

  IO.writeCommand16Transaction(0xc595);      IO.writeData16Transaction(0x34);    //power control

  IO.writeCommand16Transaction(0xd800);      IO.writeData16Transaction(0x79);    //GVDD / NGVDD setting

  IO.writeCommand16Transaction(0xd801);      IO.writeData16Transaction(0x79);    //GVDD / NGVDD setting

  IO.writeCommand16Transaction(0xc594);      IO.writeData16Transaction(0x33);    //power control

  IO.writeCommand16Transaction(0xc0a3);      IO.writeData16Transaction(0x1B);       //panel timing setting
  IO.writeCommand16Transaction(0xc582);      IO.writeData16Transaction(0x83);    //power control

  IO.writeCommand16Transaction(0xc481);      IO.writeData16Transaction(0x83);    //source driver setting

  IO.writeCommand16Transaction(0xc1a1);      IO.writeData16Transaction(0x0E);
  IO.writeCommand16Transaction(0xb3a6);      IO.writeData16Transaction(0x20);
  IO.writeCommand16Transaction(0xb3a7);      IO.writeData16Transaction(0x01);
  IO.writeCommand16Transaction(0xce80);      IO.writeData16Transaction(0x85);    // GOA VST

  IO.writeCommand16Transaction(0xce81);      IO.writeData16Transaction(0x01);  // GOA VST

  IO.writeCommand16Transaction(0xce82);      IO.writeData16Transaction(0x00);    // GOA VST

  IO.writeCommand16Transaction(0xce83);      IO.writeData16Transaction(0x84);    // GOA VST
  IO.writeCommand16Transaction(0xce84);      IO.writeData16Transaction(0x01);    // GOA VST
  IO.writeCommand16Transaction(0xce85);      IO.writeData16Transaction(0x00);    // GOA VST

  IO.writeCommand16Transaction(0xcea0);      IO.writeData16Transaction(0x18);    // GOA CLK
  IO.writeCommand16Transaction(0xcea1);      IO.writeData16Transaction(0x04);    // GOA CLK
  IO.writeCommand16Transaction(0xcea2);      IO.writeData16Transaction(0x03);    // GOA CLK
  IO.writeCommand16Transaction(0xcea3);      IO.writeData16Transaction(0x39);    // GOA CLK
  IO.writeCommand16Transaction(0xcea4);      IO.writeData16Transaction(0x00);    // GOA CLK
  IO.writeCommand16Transaction(0xcea5);      IO.writeData16Transaction(0x00);    // GOA CLK
  IO.writeCommand16Transaction(0xcea6);      IO.writeData16Transaction(0x00);    // GOA CLK
  IO.writeCommand16Transaction(0xcea7);      IO.writeData16Transaction(0x18);    // GOA CLK
  IO.writeCommand16Transaction(0xcea8);      IO.writeData16Transaction(0x03);    // GOA CLK
  IO.writeCommand16Transaction(0xcea9);      IO.writeData16Transaction(0x03);    // GOA CLK
  IO.writeCommand16Transaction(0xceaa);      IO.writeData16Transaction(0x3a);    // GOA CLK
  IO.writeCommand16Transaction(0xceab);      IO.writeData16Transaction(0x00);    // GOA CLK
  IO.writeCommand16Transaction(0xceac);      IO.writeData16Transaction(0x00);    // GOA CLK
  IO.writeCommand16Transaction(0xcead);      IO.writeData16Transaction(0x00);    // GOA CLK
  IO.writeCommand16Transaction(0xceb0);      IO.writeData16Transaction(0x18);    // GOA CLK
  IO.writeCommand16Transaction(0xceb1);      IO.writeData16Transaction(0x02);    // GOA CLK
  IO.writeCommand16Transaction(0xceb2);      IO.writeData16Transaction(0x03);    // GOA CLK
  IO.writeCommand16Transaction(0xceb3);      IO.writeData16Transaction(0x3b);    // GOA CLK
  IO.writeCommand16Transaction(0xceb4);      IO.writeData16Transaction(0x00);    // GOA CLK
  IO.writeCommand16Transaction(0xceb5);      IO.writeData16Transaction(0x00);    // GOA CLK
  IO.writeCommand16Transaction(0xceb6);      IO.writeData16Transaction(0x00);    // GOA CLK
  IO.writeCommand16Transaction(0xceb7);      IO.writeData16Transaction(0x18);    // GOA CLK
  IO.writeCommand16Transaction(0xceb8);      IO.writeData16Transaction(0x01);    // GOA CLK
  IO.writeCommand16Transaction(0xceb9);      IO.writeData16Transaction(0x03);    // GOA CLK
  IO.writeCommand16Transaction(0xceba);      IO.writeData16Transaction(0x3c);    // GOA CLK
  IO.writeCommand16Transaction(0xcebb);      IO.writeData16Transaction(0x00);    // GOA CLK
  IO.writeCommand16Transaction(0xcebc);      IO.writeData16Transaction(0x00);    // GOA CLK
  IO.writeCommand16Transaction(0xcebd);      IO.writeData16Transaction(0x00);    // GOA CLK
  IO.writeCommand16Transaction(0xcfc0);      IO.writeData16Transaction(0x01);    // GOA ECLK
  IO.writeCommand16Transaction(0xcfc1);      IO.writeData16Transaction(0x01);    // GOA ECLK
  IO.writeCommand16Transaction(0xcfc2);      IO.writeData16Transaction(0x20);    // GOA ECLK

  IO.writeCommand16Transaction(0xcfc3);      IO.writeData16Transaction(0x20);    // GOA ECLK

  IO.writeCommand16Transaction(0xcfc4);      IO.writeData16Transaction(0x00);    // GOA ECLK

  IO.writeCommand16Transaction(0xcfc5);      IO.writeData16Transaction(0x00);    // GOA ECLK

  IO.writeCommand16Transaction(0xcfc6);      IO.writeData16Transaction(0x01);    // GOA other options

  IO.writeCommand16Transaction(0xcfc7);      IO.writeData16Transaction(0x00);

  // GOA signal toggle option setting

  IO.writeCommand16Transaction(0xcfc8);      IO.writeData16Transaction(0x00);    //GOA signal toggle option setting
  IO.writeCommand16Transaction(0xcfc9);      IO.writeData16Transaction(0x00);

  //GOA signal toggle option setting

  IO.writeCommand16Transaction(0xcfd0);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcb80);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcb81);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcb82);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcb83);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcb84);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcb85);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcb86);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcb87);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcb88);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcb89);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcb90);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcb91);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcb92);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcb93);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcb94);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcb95);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcb96);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcb97);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcb98);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcb99);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcb9a);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcb9b);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcb9c);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcb9d);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcb9e);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcba0);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcba1);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcba2);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcba3);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcba4);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcba5);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcba6);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcba7);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcba8);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcba9);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbaa);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbab);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbac);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbad);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbae);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbb0);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbb1);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbb2);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbb3);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbb4);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbb5);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbb6);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbb7);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbb8);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbb9);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbc0);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbc1);      IO.writeData16Transaction(0x04);
  IO.writeCommand16Transaction(0xcbc2);      IO.writeData16Transaction(0x04);
  IO.writeCommand16Transaction(0xcbc3);      IO.writeData16Transaction(0x04);
  IO.writeCommand16Transaction(0xcbc4);      IO.writeData16Transaction(0x04);
  IO.writeCommand16Transaction(0xcbc5);      IO.writeData16Transaction(0x04);
  IO.writeCommand16Transaction(0xcbc6);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbc7);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbc8);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbc9);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbca);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbcb);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbcc);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbcd);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbce);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbd0);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbd1);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbd2);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbd3);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbd4);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbd5);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbd6);      IO.writeData16Transaction(0x04);
  IO.writeCommand16Transaction(0xcbd7);      IO.writeData16Transaction(0x04);
  IO.writeCommand16Transaction(0xcbd8);      IO.writeData16Transaction(0x04);
  IO.writeCommand16Transaction(0xcbd9);      IO.writeData16Transaction(0x04);
  IO.writeCommand16Transaction(0xcbda);      IO.writeData16Transaction(0x04);
  IO.writeCommand16Transaction(0xcbdb);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbdc);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbdd);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbde);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbe0);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbe1);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbe2);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbe3);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbe4);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbe5);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbe6);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbe7);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbe8);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbe9);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcbf0);      IO.writeData16Transaction(0xFF);
  IO.writeCommand16Transaction(0xcbf1);      IO.writeData16Transaction(0xFF);
  IO.writeCommand16Transaction(0xcbf2);      IO.writeData16Transaction(0xFF);
  IO.writeCommand16Transaction(0xcbf3);      IO.writeData16Transaction(0xFF);
  IO.writeCommand16Transaction(0xcbf4);      IO.writeData16Transaction(0xFF);
  IO.writeCommand16Transaction(0xcbf5);      IO.writeData16Transaction(0xFF);
  IO.writeCommand16Transaction(0xcbf6);      IO.writeData16Transaction(0xFF);
  IO.writeCommand16Transaction(0xcbf7);      IO.writeData16Transaction(0xFF);
  IO.writeCommand16Transaction(0xcbf8);      IO.writeData16Transaction(0xFF);
  IO.writeCommand16Transaction(0xcbf9);      IO.writeData16Transaction(0xFF);
  IO.writeCommand16Transaction(0xcc80);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcc81);      IO.writeData16Transaction(0x26);
  IO.writeCommand16Transaction(0xcc82);      IO.writeData16Transaction(0x09);
  IO.writeCommand16Transaction(0xcc83);      IO.writeData16Transaction(0x0B);
  IO.writeCommand16Transaction(0xcc84);      IO.writeData16Transaction(0x01);
  IO.writeCommand16Transaction(0xcc85);      IO.writeData16Transaction(0x25);
  IO.writeCommand16Transaction(0xcc86);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcc87);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcc88);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcc89);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcc90);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcc91);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcc92);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcc93);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcc94);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcc95);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcc96);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcc97);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcc98);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcc99);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcc9a);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcc9b);      IO.writeData16Transaction(0x26);
  IO.writeCommand16Transaction(0xcc9c);      IO.writeData16Transaction(0x0A);
  IO.writeCommand16Transaction(0xcc9d);      IO.writeData16Transaction(0x0C);
  IO.writeCommand16Transaction(0xcc9e);      IO.writeData16Transaction(0x02);
  IO.writeCommand16Transaction(0xcca0);      IO.writeData16Transaction(0x25);
  IO.writeCommand16Transaction(0xcca1);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcca2);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcca3);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcca4);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcca5);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcca6);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcca7);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcca8);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcca9);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xccaa);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xccab);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xccac);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xccad);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xccae);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xccb0);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xccb1);      IO.writeData16Transaction(0x25);
  IO.writeCommand16Transaction(0xccb2);      IO.writeData16Transaction(0x0C);
  IO.writeCommand16Transaction(0xccb3);      IO.writeData16Transaction(0x0A);
  IO.writeCommand16Transaction(0xccb4);      IO.writeData16Transaction(0x02);
  IO.writeCommand16Transaction(0xccb5);      IO.writeData16Transaction(0x26);
  IO.writeCommand16Transaction(0xccb6);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xccb7);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xccb8);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xccb9);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xccc0);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xccc1);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xccc2);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xccc3);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xccc4);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xccc5);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xccc6);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xccc7);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xccc8);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xccc9);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xccca);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xcccb);      IO.writeData16Transaction(0x25);
  IO.writeCommand16Transaction(0xcccc);      IO.writeData16Transaction(0x0B);
  IO.writeCommand16Transaction(0xcccd);      IO.writeData16Transaction(0x09);
  IO.writeCommand16Transaction(0xccce);      IO.writeData16Transaction(0x01);
  IO.writeCommand16Transaction(0xccd0);      IO.writeData16Transaction(0x26);
  IO.writeCommand16Transaction(0xccd1);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xccd2);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xccd3);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xccd4);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xccd5);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xccd6);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xccd7);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xccd8);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xccd9);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xccda);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xccdb);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xccdc);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xccdd);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xccde);      IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0x3A00);      IO.writeData16Transaction(0x55);

  IO.writeCommand16Transaction(0x1100);
  delay(100);
  IO.writeCommand16Transaction(0x2900);
  delay(50);
  IO.writeCommand16Transaction(0x2C00);
  IO.writeCommand16Transaction(0x2A00);     IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0x2A01);     IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0x2A02);     IO.writeData16Transaction(0x01);
  IO.writeCommand16Transaction(0x2A03);     IO.writeData16Transaction(0xe0);
  IO.writeCommand16Transaction(0x2B00);     IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0x2B01);     IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0x2B02);     IO.writeData16Transaction(0x03);
  IO.writeCommand16Transaction(0x2B03);     IO.writeData16Transaction(0x20);
}

void GxCTRL_OTM8009A::setWindowAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
  IO.writeCommand16(0x2A00);
  IO.writeData16(x0 >> 8);
  IO.writeCommand16(0x2A01);
  IO.writeData16(x0 & 0x00ff);
  IO.writeCommand16(0x2A02);
  IO.writeData16(x1 >> 8);
  IO.writeCommand16(0x2A03);
  IO.writeData16(x1 & 0x00ff);
  IO.writeCommand16(0x2B00);
  IO.writeData16(y0 >> 8);
  IO.writeCommand16(0x2B01);
  IO.writeData16(y0 & 0x00ff);
  IO.writeCommand16(0x2B02);
  IO.writeData16(y1 >> 8);
  IO.writeCommand16(0x2B03);
  IO.writeData16(y1 & 0x00ff);
  IO.writeCommand16(0x2C00);
}

void GxCTRL_OTM8009A::setRotation(uint8_t r)
{
  IO.startTransaction();
  IO.writeCommand16(OTM8009A_MADCTL);
  switch (r & 3)
  {
    case 0:
      IO.writeData(0);
      break;
    case 1:
      IO.writeData(OTM8009A_MADCTL_MX | OTM8009A_MADCTL_MV);
      break;
    case 2:
      IO.writeData(OTM8009A_MADCTL_MX | OTM8009A_MADCTL_MY);
      break;
    case 3:
      IO.writeData(OTM8009A_MADCTL_MY | OTM8009A_MADCTL_MV);
      break;
  }
  IO.endTransaction();
}


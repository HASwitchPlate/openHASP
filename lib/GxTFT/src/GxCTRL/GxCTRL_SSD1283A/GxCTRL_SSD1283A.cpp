// created by Jean-Marc Zingg to be the GxCTRL_SSD1283A class for the GxTFT library
// code extracts taken from https://github.com/lcdwiki/LCDWIKI_SPI
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// note: my only SSD1283A display is write only, read methods of this class don't work

#include "GxCTRL_SSD1283A.h"

uint32_t GxCTRL_SSD1283A::readID()
{
  return 0;
}

uint32_t GxCTRL_SSD1283A::readRegister(uint8_t nr, uint8_t index, uint8_t bytes)
{
  return 0;
}

uint16_t GxCTRL_SSD1283A::readPixel(uint16_t x, uint16_t y)
{
  return 0;
}

void GxCTRL_SSD1283A::readRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t* data)
{
}

void GxCTRL_SSD1283A::init()
{
  _rotation = 0;
  IO.startTransaction();
  IO.writeCommand(0x10);
  IO.writeData(0x2F);
  IO.writeData(0x8E);
  IO.writeCommand(0x11);
  IO.writeData(0x00);
  IO.writeData(0x0C);
  IO.writeCommand(0x07);
  IO.writeData(0x00);
  IO.writeData(0x21);
  IO.writeCommand(0x28);
  IO.writeData(0x00);
  IO.writeData(0x06);
  IO.writeCommand(0x28);
  IO.writeData(0x00);
  IO.writeData(0x05);
  IO.writeCommand(0x27);
  IO.writeData(0x05);
  IO.writeData(0x7F);
  IO.writeCommand(0x29);
  IO.writeData(0x89);
  IO.writeData(0xA1);
  IO.writeCommand(0x00);
  IO.writeData(0x00);
  IO.writeData(0x01);
  delay(100);
  IO.writeCommand(0x29);
  IO.writeData(0x80);
  IO.writeData(0xB0);
  delay(30);
  IO.writeCommand(0x29);
  IO.writeData(0xFF);
  IO.writeData(0xFE);
  IO.writeCommand(0x07);
  IO.writeData(0x02);
  IO.writeData(0x23);
  delay(30);
  IO.writeCommand(0x07);
  IO.writeData(0x02);
  IO.writeData(0x33);
  IO.writeCommand(0x01);
  IO.writeData(0x21);
  IO.writeData(0x83);
  IO.writeCommand(0x03);
  IO.writeData(0x68);
  IO.writeData(0x30);
  IO.writeCommand(0x2F);
  IO.writeData(0xFF);
  IO.writeData(0xFF);
  IO.writeCommand(0x2C);
  IO.writeData(0x80);
  IO.writeData(0x00);
  IO.writeCommand(0x27);
  IO.writeData(0x05);
  IO.writeData(0x70);
  IO.writeCommand(0x02);
  IO.writeData(0x03);
  IO.writeData(0x00);
  IO.writeCommand(0x0B);
  IO.writeData(0x58);
  IO.writeData(0x0C);
  IO.writeCommand(0x12);
  IO.writeData(0x06);
  IO.writeData(0x09);
  IO.writeCommand(0x13);
  IO.writeData(0x31);
  IO.writeData(0x00);
  IO.endTransaction();
}

void GxCTRL_SSD1283A::setWindowAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
  switch (_rotation)
  {
    case 0:
      IO.writeCommand(0x44);
      IO.writeData(x1 + 2);
      IO.writeData(x0 + 2);
      IO.writeCommand(0x45);
      IO.writeData(y1 + 2);
      IO.writeData(y0 + 2);
      IO.writeCommand(0x21);
      IO.writeData(y0 + 2);
      IO.writeData(x0 + 2);
      break;
    case 1:
      IO.writeCommand(0x44);
      IO.writeData(_tft_height - y0 + 1);
      IO.writeData(_tft_height - y1 + 1);
      IO.writeCommand(0x45);
      IO.writeData(_tft_width - x0 - 1);
      IO.writeData(_tft_width - x1 - 1);
      IO.writeCommand(0x21);
      IO.writeData(_tft_width - x0 - 1);
      IO.writeData(_tft_height - y0 + 1);
      break;
    case 2:
      IO.writeCommand(0x44);
      IO.writeData(_tft_width - x0 + 1);
      IO.writeData(_tft_width - x1 + 1);
      IO.writeCommand(0x45);
      IO.writeData(_tft_height - y0 + 1);
      IO.writeData(_tft_height - y1 + 1);
      IO.writeCommand(0x21);
      IO.writeData(_tft_height - y0 + 1);
      IO.writeData(_tft_width - x0 + 1);
      break;
    case 3:
      IO.writeCommand(0x44);
      IO.writeData(y1 + 2);
      IO.writeData(y0 + 2);
      IO.writeCommand(0x45);
      IO.writeData(x1);
      IO.writeData(x0);
      IO.writeCommand(0x21);
      IO.writeData(x0);
      IO.writeData(y0 + 2);
      break;
  }
  IO.writeCommand(0x22);
}

void GxCTRL_SSD1283A::setRotation(uint8_t r)
{
  _rotation = r;
  IO.startTransaction();
  switch (r & 3)
  {
    case 0:
    case 2:
      IO.writeCommand(0x01);
      IO.writeData(0x21);
      IO.writeData(0x83);
      IO.writeCommand(0x03);
      IO.writeData(0x68);
      IO.writeData(0x30);
      break;
    case 1:
    case 3:
      IO.writeCommand(0x01);
      IO.writeData(0x22);
      IO.writeData(0x83);
      IO.writeCommand(0x03);
      IO.writeData(0x68);
      IO.writeData(0x38);
      break;
  }
  IO.endTransaction();
}

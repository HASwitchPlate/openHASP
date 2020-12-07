// created by Jean-Marc Zingg to be the GxCTRL_ILI9325D class for the GxTFT library
// code extracts taken from http://www.rinkydinkelectronics.com/download.php?f=UTFT.zip
// code extracts taken from https://github.com/adafruit/TFTLCD-Library
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE

#ifndef _GxCTRL_ILI9325D_H_
#define _GxCTRL_ILI9325D_H_

#include "../GxCTRL.h"

class GxCTRL_ILI9325D : public GxCTRL
{
  public:
    GxCTRL_ILI9325D(GxIO& io) : GxCTRL(io), _tft_width(240), _tft_height(320) {};
    GxCTRL_ILI9325D(GxIO& io, uint16_t tft_width, uint16_t tft_height) : GxCTRL(io), _tft_width(tft_width), _tft_height(tft_height) {};
    const char* name = "GxCTRL_ILI9325D";
    const uint32_t ID = 0x9325;
    uint32_t readID();
    uint32_t readRegister(uint8_t nr, uint8_t index = 0, uint8_t bytes = 1);
    uint16_t readPixel(uint16_t x, uint16_t y);
    void     readRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t* data);
    void init();
    void setWindowAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
    void setRotation(uint8_t r);
  private:
    void LCD_Write_COM_DATA(uint8_t com, uint16_t data);
    uint8_t _rotation;
    uint16_t _tft_width, _tft_height; // physical
};

#define GxCTRL_Class GxCTRL_ILI9325D

#endif



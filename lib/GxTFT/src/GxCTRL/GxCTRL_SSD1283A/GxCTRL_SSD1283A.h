// created by Jean-Marc Zingg to be the GxCTRL_SSD1283A class for the GxTFT library
// code extracts taken from https://github.com/lcdwiki/LCDWIKI_SPI
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// note: my only SSD1283A display is write only, read methods of this class don't work

#ifndef _GxCTRL_SSD1283A_H_
#define _GxCTRL_SSD1283A_H_

#include "../GxCTRL.h"

class GxCTRL_SSD1283A : public GxCTRL
{
  public:
    GxCTRL_SSD1283A(GxIO& io) : GxCTRL(io), _tft_width(130), _tft_height(130) {};
    GxCTRL_SSD1283A(GxIO& io, uint16_t tft_width, uint16_t tft_height) : GxCTRL(io), _tft_width(tft_width), _tft_height(tft_height) {};
    const char* name = "GxCTRL_SSD1283A";
    const uint32_t ID = 0x1283;
    uint32_t readID();
    uint32_t readRegister(uint8_t nr, uint8_t index = 0, uint8_t bytes = 1);
    uint16_t readPixel(uint16_t x, uint16_t y);
    void     readRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t* data);
    void init();
    void setWindowAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
    void setRotation(uint8_t r);
  private:
    uint8_t _rotation;
    uint16_t _tft_width, _tft_height; // physical
};

#define GxCTRL_Class GxCTRL_SSD1283A

#endif

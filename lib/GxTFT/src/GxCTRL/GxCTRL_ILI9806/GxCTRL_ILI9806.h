// created by Jean-Marc Zingg to be the GxCTRL_ILI9806 class for the GxTFT library
// code extracts taken from code and documentation from Ruijia Industry (Tiky_LCD.h, Tiky_LCD.c)
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// note: readRect does not work correctly with my only ILI9806 display (pixel sequence)
//       workaround added

#ifndef _GxCTRL_ILI9806_H_
#define _GxCTRL_ILI9806_H_

#include "../GxCTRL.h"

class GxCTRL_ILI9806 : public GxCTRL
{
  public:
    GxCTRL_ILI9806(GxIO& io) : GxCTRL(io) {};
    const char* name = "GxCTRL_ILI9806";
    const uint32_t ID = 0x9806;
    uint32_t readID();
    uint32_t readRegister(uint8_t nr, uint8_t index = 0, uint8_t bytes = 1);
    uint16_t readPixel(uint16_t x, uint16_t y);
    void     readRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t* data);
    void init();
    void setWindowAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
    void setRotation(uint8_t r);
};

#define GxCTRL_Class GxCTRL_ILI9806

#endif


// created by Jean-Marc Zingg to be the GxCTRL_HX8357C class for the GxTFT library
// code extracts taken from https://github.com/Bodmer/TFT_HX8357
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// note: untested, and read functions are untested; I have no HX8357 display to test with

#ifndef _GxCTRL_HX8357C_H_
#define _GxCTRL_HX8357C_H_

#include "../GxCTRL.h"

class GxCTRL_HX8357C : public GxCTRL
{
  public:
    GxCTRL_HX8357C(GxIO& io) : GxCTRL(io) {};
    const char* name = "GxCTRL_HX8357C";
    const uint32_t ID = 0x8357;
    uint32_t readID();
    uint32_t readRegister(uint8_t nr, uint8_t index = 0, uint8_t bytes = 1);
    void init();
    void setWindowAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
    void setRotation(uint8_t r);
};

#define GxCTRL_Class GxCTRL_HX8357C

#endif



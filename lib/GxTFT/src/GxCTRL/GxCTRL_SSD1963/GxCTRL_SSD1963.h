// created by Jean-Marc Zingg to be the GxCTRL_SSD1963 class for the GxTFT library
// code extracts taken from http://www.rinkydinkelectronics.com/download.php?f=UTFT.zip
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE

#ifndef _GxCTRL_SSD1963_H_
#define _GxCTRL_SSD1963_H_

#include "../GxCTRL.h"

class GxCTRL_SSD1963 : public GxCTRL
{
  public:
    GxCTRL_SSD1963(GxIO& io) : GxCTRL(io), physical_width(800), physical_height(480) {};
    GxCTRL_SSD1963(GxIO& io, uint16_t phy_w, uint16_t phy_h) : GxCTRL(io), physical_width(phy_w), physical_height(phy_w) {};
    const char* name = "GxCTRL_SSD1963";
    const uint32_t ID = 0x1963;
    uint32_t readID();
    uint32_t readRegister(uint8_t nr, uint8_t index = 0, uint8_t bytes = 1);
    uint16_t readPixel(uint16_t x, uint16_t y);
    void     readRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t* data);
    void init();
    void setWindowAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
    void setRotation(uint8_t r);
  private:
    uint8_t rotation;
    uint16_t physical_width, physical_height;
};

#define GxCTRL_Class GxCTRL_SSD1963

#endif


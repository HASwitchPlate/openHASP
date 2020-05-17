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

#ifndef _GxCTRL_OTM8009A_H_
#define _GxCTRL_OTM8009A_H_

#include "../GxCTRL.h"

class GxCTRL_OTM8009A : public GxCTRL
{
  public:
    GxCTRL_OTM8009A(GxIO& io) : GxCTRL(io) {};
    const char* name = "GxCTRL_OTM8009A";
    const uint32_t ID = 0x8009A;
    uint32_t readID();
    uint32_t readRegister(uint8_t nr, uint8_t index = 0, uint8_t bytes = 1);
    uint16_t readPixel(uint16_t x, uint16_t y);
    void     readRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t* data);
    void init();
    void setWindowAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
    void setRotation(uint8_t r);
};

#define GxCTRL_Class GxCTRL_OTM8009A

#endif


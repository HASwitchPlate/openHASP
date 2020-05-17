// created by Jean-Marc Zingg to be the GxIO_STM32F407ZGM4_P16 io class for the GxTFT library
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// this is a wiring for the TFT  connector, e.g. of
// https://www.aliexpress.com/item/STM32F407ZGT6-Development-Board-ARM-M4-STM32F4-cortex-M4-core-Board-Compatibility-Multiple-Extension/32795142050.html
// and the matching TFTs of the same offer
//
// and e.g. for direct matching display
// https://www.aliexpress.com/item/Smart-Electronics-3-5-inch-TFT-Touch-Screen-LCD-Module-Display-320-480-ILI9486-with-PCB/32586941686.html
//
// this version is for use with Arduino package STM32GENERIC, board "BLACK F407VE/ZE/ZG boards".
// Specific Board "BLACK F407ZG (M4 DEMO)"
// https://github.com/danieleff/STM32GENERIC

#ifndef _GxIO_STM32F407ZGM4_P16_H_
#define _GxIO_STM32F407ZGM4_P16_H_

#include "../../GxIO.h"

#if defined(ARDUINO_ARCH_STM32) && (defined(STM32F407VE) || defined(STM32F407ZE) || defined(STM32F407ZG))

class GxIO_STM32F407ZGM4_P16 : public GxIO
{
  public:
    GxIO_STM32F407ZGM4_P16(bool bl_active_high = true);
    const char* name = "GxIO_STM32F407ZGM4_P16";
    void reset();
    void init();
    uint8_t readDataTransaction();
    uint16_t readData16Transaction();
    uint8_t readData();
    uint16_t readData16();
    uint32_t readRawData32(uint8_t part); // debug purpose
    void writeCommandTransaction(uint8_t c);
    void writeCommand16Transaction(uint16_t c);
    void writeDataTransaction(uint8_t d);
    void writeData16Transaction(uint16_t d, uint32_t num = 1);
    void writeCommand(uint8_t c);
    void writeCommand16(uint16_t c);
    void writeData(uint8_t d);
    void writeData(uint8_t* d, uint32_t num);
    void writeData16(uint16_t d, uint32_t num = 1);
    void writeAddrMSBfirst(uint16_t d);
    void startTransaction();
    void endTransaction();
    void selectRegister(bool rs_low); // for generalized readData & writeData (RA8875)
    void setBackLight(bool lit);
  private:
    int8_t _cs, _rs, _rst, _wr, _rd, _bl; // Control lines
    bool _bl_active_high;
};

#define GxIO_Class GxIO_STM32F407ZGM4_P16

#endif

#endif


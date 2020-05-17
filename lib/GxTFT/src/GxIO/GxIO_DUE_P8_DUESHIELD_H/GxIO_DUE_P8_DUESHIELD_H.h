// created by Jean-Marc Zingg to be the GxIO_DUE_P8_DUESHIELD_H io class for the GxTFT library
// code extracts taken from https://github.com/Bodmer/TFT_HX8357_Due
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// for 8 bit parallel displays on shields or on adapter shields for DUE, e.g. CTE TFT LCD/SD Shield for Arduino Due
// e.g. https://www.aliexpress.com/item/New-TFT-SD-Shield-for-Arduino-DUE-TFT-LCD-Module-SD-Card-Adapter-2-8-3/32709157722.html
// for read functions to work, a connection from an Arduino pin to the LCD_RD signal needs to be made
// tested with 2.8" ILI9325 TFT, e.g. https://www.aliexpress.com/item/TFT-2-8-LCD-Touch-Screen-module-3-3V-with-SD-and-MicroSD-card/32711829802.html
//
// IMPORTANT : make sure the jumpers are set correctly for the supply voltages of your TFT, do measure for safety!

#ifndef _GxIO_DUE_P8_DUESHIELD_H_H_
#define _GxIO_DUE_P8_DUESHIELD_H_H_

#include "../GxIO.h"

#if defined(ARDUINO_ARCH_SAM)

class GxIO_DUE_P8_DUESHIELD_H : public GxIO
{
  public:
    GxIO_DUE_P8_DUESHIELD_H();
    const char* name = "GxIO_DUE_P8_DUESHIELD_H";
    void reset();
    void init();
    uint8_t readDataTransaction();
    uint16_t readData16Transaction();
    uint8_t readData();
    uint16_t readData16();
    uint32_t readRawData32(uint8_t part); // debug purpose
    void writeCommandTransaction(uint8_t c);
    void writeDataTransaction(uint8_t d);
    void writeData16Transaction(uint16_t d, uint32_t num = 1);
    void writeCommand(uint8_t c);
    void writeData(uint8_t d);
    void writeData(uint8_t* d, uint32_t num);
    void writeData16(uint16_t d, uint32_t num = 1);
    void writeAddrMSBfirst(uint16_t d);
    void startTransaction();
    void endTransaction();
    void selectRegister(bool rs_low); // for generalized readData & writeData (RA8875)
    void setBackLight(bool lit);
  private:
    void setDataPins(uint8_t mode);
    int8_t _cs, _rs, _rst, _wr, _rd, _bl; // Control lines
};

#define GxIO_Class GxIO_DUE_P8_DUESHIELD_H

#endif

#endif


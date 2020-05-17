// created by Jean-Marc Zingg to be the GxIO_UNO_P8_SHIELD io class for the GxTFT library
// code extracts taken from https://github.com/Bodmer/TFT_HX8357
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// for 8 bit parallel displays on shields or on adapter shields for UNO, e.g. mcufriend 8bit tft shields

#ifndef _GxIO_UNO_P8_SHIELD_H_
#define _GxIO_UNO_P8_SHIELD_H_

#include "../GxIO.h"

#if defined(__AVR_ATmega328P__)

class GxIO_UNO_P8_SHIELD : public GxIO
{
  public:
    GxIO_UNO_P8_SHIELD();
    const char* name = "GxIO_UNO_P8_SHIELD";
    void reset();
    void init();
    uint8_t readDataTransaction();
    uint16_t readData16Transaction();
    uint8_t readData();
    uint16_t readData16();
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
    int8_t _cs, _rs, _rst, _wr, _rd, _bl; // Control lines
};

#define GxIO_Class GxIO_UNO_P8_SHIELD

#endif

#endif



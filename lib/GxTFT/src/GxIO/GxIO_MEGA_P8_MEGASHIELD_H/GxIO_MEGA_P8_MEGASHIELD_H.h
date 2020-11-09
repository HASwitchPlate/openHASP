// created by Jean-Marc Zingg to be the GxIO_MEGA_P8_MEGASHIELD_H io class for the GxTFT library
// code extracts taken from https://github.com/Bodmer/TFT_HX8357
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// for 8 bit parallel displays on shields or on adapter shields for MEGA, using PORTA to high byte

// read functions work on my modified INHAOS MEGA shield (remove R7, add 1k from there to pin 43)
// I had to add 3k9 pull-up resistors to the data lines on the MEGA side of the series resistors

#ifndef _GxIO_MEGA_P8_MEGASHIELD_H_H_
#define _GxIO_MEGA_P8_MEGASHIELD_H_H_

#include "../GxIO.h"

#if defined(__AVR_ATmega2560__)

class GxIO_MEGA_P8_MEGASHIELD_H : public GxIO
{
  public:
    GxIO_MEGA_P8_MEGASHIELD_H();
    const char* name = "GxIO_MEGA_P8_MEGASHIELD_H";
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

#define GxIO_Class GxIO_MEGA_P8_MEGASHIELD_H

#endif

#endif



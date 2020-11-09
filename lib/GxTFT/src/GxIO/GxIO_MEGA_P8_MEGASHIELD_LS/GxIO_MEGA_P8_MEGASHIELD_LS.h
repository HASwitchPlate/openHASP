// created by Jean-Marc Zingg to be the GxIO_MEGA_P8_MEGASHIELD_LS io class for the GxTFT library
// code extracts taken from https://github.com/Bodmer/TFT_HX8357
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// for 8 bit parallel displays with latch on shields or on adapter shields for MEGA/DUE, e.g. INHAOS_LCD_2000_9225 on INHAOS MEGA shield

// read functions do not work, even with resistor from pin 43 to RD

#ifndef _GxIO_MEGA_P8_MEGASHIELD_LS_H_
#define _GxIO_MEGA_P8_MEGASHIELD_LS_H_

#include "../GxIO.h"

#if defined(__AVR_ATmega2560__)

class GxIO_MEGA_P8_MEGASHIELD_LS : public GxIO
{
  public:
    GxIO_MEGA_P8_MEGASHIELD_LS(uint8_t latch_strobe_pin = 42);
    const char* name = "GxIO_MEGA_P8_MEGASHIELD_LS";
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
    int8_t _ls; // latch strobe
    volatile uint8_t* _lsport;
    uint8_t _lsbit;
};

#define GxIO_Class GxIO_MEGA_P8_MEGASHIELD_LS

#endif

#endif



// created by Jean-Marc Zingg to be the GxIO_DUE_P16_HVGASHIELD io class for the GxTFT library
// code extracts taken from https://github.com/Bodmer/TFT_HX8357_Due
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// for 16 bit parallel displays on shields for MEGA, usually marked HVGA 480X320 3.2" TFTLCD Shield for Arduino Mega2560
// this is an example how to achieve an arbitrary pin mapping in software for Arduino Due.
//
// for optimal performance use Bodmer's TFT_HX8357_Due library instead.

#ifndef _GxIO_DUE_P16_HVGASHIELD_H_
#define _GxIO_DUE_P16_HVGASHIELD_H_

#include "../GxIO.h"

#if defined(ARDUINO_ARCH_SAM)

class GxIO_DUE_P16_HVGASHIELD : public GxIO
{
  public:
    GxIO_DUE_P16_HVGASHIELD();
    const char* name = "GxIO_DUE_P16_HVGASHIELD";
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
    void setDataPins(uint8_t mode);
    int8_t _cs, _rs, _rst, _wr, _rd, _bl; // Control lines
};

#define GxIO_Class GxIO_DUE_P16_HVGASHIELD

#endif

#endif

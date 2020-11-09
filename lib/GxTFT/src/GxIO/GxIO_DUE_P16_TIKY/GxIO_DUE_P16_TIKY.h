// created by Jean-Marc Zingg to be the GxIO_DUE_P16_TIKY io class for the GxTFT library
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// this is a special wiring for
// https://www.aliexpress.com/item/5-0-inch-HD-IPS-TFT-LCD-module-resistance-touch-with-PCB-adapter-board-854-480/32666829945.html
//
// for pin mapping see http://forum.arduino.cc/index.php?topic=366304.msg3190917#msg3190917
//

#ifndef _GxIO_DUE_P16_TIKY_H_
#define _GxIO_DUE_P16_TIKY_H_

#include "../GxIO.h"

#if defined(ARDUINO_ARCH_SAM)

class GxIO_DUE_P16_TIKY : public GxIO
{
  public:
    GxIO_DUE_P16_TIKY();
    const char* name = "GxIO_DUE_P16_TIKY";
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

#define GxIO_Class GxIO_DUE_P16_TIKY

#endif

#endif

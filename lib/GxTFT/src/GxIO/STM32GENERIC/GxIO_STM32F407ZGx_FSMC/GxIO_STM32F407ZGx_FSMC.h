// created by Jean-Marc Zingg to be the GxIO_STM32F407ZGM4_FSMC io class for the GxTFT library
// adapted by fvanroie to work on STM32F407ZGT6 board
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// this is the io class for STM32F407ZGM4 board with FMSC TFT connector, e.g. for 
// https://www.aliexpress.com/item/STM32-ARM-Cortex-M4-STM32F407ZGT6-development-board-STM32F4-core-board/32827402179.html?spm=a2g0s.9042311.0.0.49b14c4dUViVg4
// and the matching TFTs of the same offer
//
// and e.g. for direct matching display
// https://www.aliexpress.com/item/STM32F407ZET6-development-board-M4-STM32F4-core-board-arm-development-board-cortex-M4/32689262341.html?spm=a2g0s.9042311.0.0.49b14c4dUViVg4
// https://www.aliexpress.com/item/32662835059.html?spm=a2g0o.cart.0.0.56f03c00mB9alZ&mp=1
//
// for pin information see the backside of the TFT, for the data pin to port pin mapping see FSMC pin table STM32F407V doc.
//
// this io class can be used with or adapted to other STM32F407V/Z processor boards with FSMC TFT connector.


#ifndef _GxIO_STM32F407ZGX_FSMC_H_
#define _GxIO_STM32F407ZGX_FSMC_H_

#include "../../GxIO.h"

#if defined(ARDUINO_ARCH_STM32) && (defined(STM32F407VE) || defined(STM32F407ZE) || defined(STM32F407ZG))

class GxIO_STM32F407ZGx_FSMC : public GxIO
{
  public:
    GxIO_STM32F407ZGx_FSMC(bool bl_active_high = true);
    const char* name = "GxIO_STM32F407ZGx_FSMC";
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

#define GxIO_Class GxIO_STM32F407ZGx_FSMC

#endif

#endif


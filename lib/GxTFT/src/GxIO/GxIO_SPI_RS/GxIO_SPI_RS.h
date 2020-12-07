// created by Jean-Marc Zingg to be the GxIO_SPI_RS io class for the GxTFT library
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE

#ifndef _GxIO_SPI_RS_H_
#define _GxIO_SPI_RS_H_

#include <SPI.h>
#include "../GxIO.h"

#define GxIO_SPI_RS_defaultFrequency 16000000

#if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD) || defined(ESP8266) || defined(ESP32)

class GxIO_SPI_RS : public GxIO
{
  public:
    GxIO_SPI_RS(SPIClass& spi, int8_t cs, int8_t dc, int8_t rst = -1, int8_t bl = -1);
    const char* name = "GxIO_SPI_RS";
    void reset();
    void init();
    void setFrequency(uint32_t freq); // for SPI
    void setClockDivider(uint32_t clockDiv); // for SPI
    uint8_t transferTransaction(uint8_t d);
    uint16_t transfer16Transaction(uint16_t d);
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
  protected:
#if defined(ESP8266)
    inline void setDataBits(uint16_t bits);
#endif
    uint8_t transfer(uint8_t data, bool rs_data);
    uint16_t transfer16(uint16_t data, bool rs_data);
    SPIClass& IOSPI;
    int8_t _cs, _rst, _bl; // Control lines
};

#define GxIO_Class GxIO_SPI_RS

#endif

#endif


// created by Jean-Marc Zingg to be the GxIO_SPI_RS io class for the GxTFT library
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//

#include "GxIO_SPI_RS.h"

#if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD) || defined(ESP8266) || defined(ESP32)

GxIO_SPI_RS::GxIO_SPI_RS(SPIClass& spi, int8_t cs, int8_t dc, int8_t rst, int8_t bl) : IOSPI(spi)
{
  _cs   = cs;
  _rst  = rst;
  _bl   = bl;
}

void GxIO_SPI_RS::reset()
{
  if (_rst >= 0)
  {
    delay(20);
    digitalWrite(_rst, LOW);
    delay(20);
    digitalWrite(_rst, HIGH);
    delay(200);
  }
}

void GxIO_SPI_RS::init()
{
  if (_cs >= 0)
  {
    digitalWrite(_cs, HIGH);
    pinMode(_cs, OUTPUT);
  }
  if (_rst >= 0)
  {
    digitalWrite(_rst, HIGH);
    pinMode(_rst, OUTPUT);
  }
  if (_bl >= 0)
  {
    digitalWrite(_bl, HIGH);
    pinMode(_bl, OUTPUT);
  }
  reset();
  IOSPI.begin();
  IOSPI.setDataMode(SPI_MODE0);
  IOSPI.setBitOrder(MSBFIRST);
  setFrequency(GxIO_SPI_RS_defaultFrequency);
#if defined(ARDUINO_ARCH_SAMD)
  while(SERCOM1->SPI.SYNCBUSY.bit.ENABLE);
  SERCOM1->SPI.CTRLA.bit.ENABLE = 0;
  SERCOM1->SPI.CTRLB.reg = SERCOM_SPI_CTRLB_CHSIZE(SPI_CHAR_SIZE_9_BITS) | SERCOM_SPI_CTRLB_RXEN;
  SERCOM1->SPI.CTRLA.bit.ENABLE = 1;
  while(SERCOM1->SPI.SYNCBUSY.bit.ENABLE);
#endif
}

void GxIO_SPI_RS::setFrequency(uint32_t freq)
{
  //freq = 4000000; // for debug or analyzer
#if defined(ESP8266) || defined(ESP32)
  IOSPI.setFrequency(freq);
#elif defined(SPI_HAS_TRANSACTION)
  SPISettings settings(freq, MSBFIRST, SPI_MODE0);
  IOSPI.beginTransaction(settings);
  IOSPI.endTransaction();
#else
  // keep the SPI default (should be 4MHz)
#endif
}

void GxIO_SPI_RS::setClockDivider(uint32_t clockDiv)
{
  IOSPI.setClockDivider(clockDiv);
}

#if defined(ARDUINO_ARCH_SAM)

uint8_t GxIO_SPI_RS::transfer(uint8_t data, bool rs_data)
{
  uint32_t ch = BOARD_PIN_TO_SPI_CHANNEL(BOARD_SPI_DEFAULT_SS);
  SPI_INTERFACE->SPI_CSR[ch] |= 0x10; // 9bits
  uint32_t d = data | (rs_data << 8) | SPI_PCS(ch) | SPI_TDR_LASTXFER;
  //uint32_t d = (data << 1)| rs_data | SPI_PCS(ch) | SPI_TDR_LASTXFER;
  while ((SPI_INTERFACE->SPI_SR & SPI_SR_TDRE) == 0);
  SPI_INTERFACE->SPI_TDR = d;
  while ((SPI_INTERFACE->SPI_SR & SPI_SR_RDRF) == 0);
  d = SPI_INTERFACE->SPI_RDR;
  return (d >> 1) & 0xFF;
}

#endif

#if defined(ARDUINO_ARCH_SAMD)

uint8_t GxIO_SPI_RS::transfer(uint8_t data, bool rs_data)
{
  SERCOM1->SPI.DATA.bit.DATA = (rs_data << 8) | data;
  while (SERCOM1->SPI.INTFLAG.bit.RXC == 0);
  uint32_t d = SERCOM1->SPI.DATA.bit.DATA;
  return (d >> 1) & 0xFF;
}

#endif

#if defined(ESP32)

//#include "soc/spi_struct.h"
#include "spi_struct_type.h"

struct spi_struct_t
{
  spi_dev_t * dev;
#if !CONFIG_DISABLE_HAL_LOCKS
  xSemaphoreHandle lock;
#endif
  uint8_t num;
};

uint8_t GxIO_SPI_RS::transfer(uint8_t data, bool rs_data)
{
  spi_t* spi = IOSPI.bus();
  if (!spi) return 0;
  //SPI_MUTEX_LOCK();
  spi->dev->mosi_dlen.usr_mosi_dbitlen = 8;
  spi->dev->miso_dlen.usr_miso_dbitlen = 7;
  spi->dev->data_buf[0] = (data >> 1) | (data << 15) | (rs_data << 7);
  spi->dev->cmd.usr = 1;
  while (spi->dev->cmd.usr);
  data = spi->dev->data_buf[0] & 0xFF;
  //SPI_MUTEX_UNLOCK();
  return data;
}

#endif

#if defined(ESP8266)

inline void GxIO_SPI_RS::setDataBits(uint16_t bits)
{
  const uint32_t mask = ~((SPIMMOSI << SPILMOSI) | (SPIMMISO << SPILMISO));
  bits--;
  SPI1U1 = ((SPI1U1 & mask) | ((bits << SPILMOSI) | (bits << SPILMISO)));
}

uint8_t GxIO_SPI_RS::transfer(uint8_t data, bool rs_data)
{
  while (SPI1CMD & SPIBUSY) {}
  // low byte first, ms bit first!; CMD : dc low -> first bit 0, DATA : dc high -> first bit high
  setDataBits(9);
  SPI1W0 = (data >> 1) | (data << 15) | (rs_data << 7);
  SPI1CMD |= SPIBUSY;
  while (SPI1CMD & SPIBUSY) {}
  uint16_t rv = SPI1W0;
  return uint8_t(rv & 0xFF);
}

#endif

uint16_t GxIO_SPI_RS::transfer16(uint16_t data, bool rs_data)
{
  uint16_t rv = transfer(data >> 8, rs_data);
  rv <<= 8;
  rv |= transfer(data & 0xFF, rs_data);
  return rv;
}

uint8_t GxIO_SPI_RS::transferTransaction(uint8_t d)
{
  if (_cs >= 0) digitalWrite(_cs, LOW);
  uint8_t rv = transfer(d, true);
  if (_cs >= 0) digitalWrite(_cs, HIGH);
  return rv;
}

uint16_t GxIO_SPI_RS::transfer16Transaction(uint16_t d)
{
  if (_cs >= 0) digitalWrite(_cs, LOW);
  uint16_t rv = transfer16(d, true);
  if (_cs >= 0) digitalWrite(_cs, HIGH);
  return rv;
}

uint8_t GxIO_SPI_RS::readDataTransaction()
{
  if (_cs >= 0) digitalWrite(_cs, LOW);
  uint8_t rv = transfer(0xFF, true);
  if (_cs >= 0) digitalWrite(_cs, HIGH);
  return rv;
}

uint16_t GxIO_SPI_RS::readData16Transaction()
{
  if (_cs >= 0) digitalWrite(_cs, LOW);
  uint16_t rv = transfer16(0xFFFF, true);
  if (_cs >= 0) digitalWrite(_cs, HIGH);
  return rv;
}

uint8_t GxIO_SPI_RS::readData()
{
  return transfer(0xFF, true);
}

uint16_t GxIO_SPI_RS::readData16()
{
  return transfer16(0xFFFF, true);
}

void GxIO_SPI_RS::writeCommandTransaction(uint8_t c)
{
  if (_cs >= 0) digitalWrite(_cs, LOW);
  transfer(c, false);
  if (_cs >= 0) digitalWrite(_cs, HIGH);
}

void GxIO_SPI_RS::writeDataTransaction(uint8_t d)
{
  if (_cs >= 0) digitalWrite(_cs, LOW);
  transfer(d, true);
  if (_cs >= 0) digitalWrite(_cs, HIGH);
}

void GxIO_SPI_RS::writeData16Transaction(uint16_t d, uint32_t num)
{
  if (_cs >= 0) digitalWrite(_cs, LOW);
  writeData16(d, num);
  if (_cs >= 0) digitalWrite(_cs, HIGH);
}

void GxIO_SPI_RS::writeCommand(uint8_t c)
{
  transfer(c, false);
}

void GxIO_SPI_RS::writeData(uint8_t d)
{
  transfer(d, true);
}

void GxIO_SPI_RS::writeData(uint8_t* d, uint32_t num)
{
  while (num > 0)
  {
    transfer(*d, true);
    d++;
    num--;
  }
}

void GxIO_SPI_RS::writeData16(uint16_t d, uint32_t num)
{
  while (num > 0)
  {
    transfer16(d, true);
    num--;
  }
}

void GxIO_SPI_RS::writeAddrMSBfirst(uint16_t d)
{
  transfer(d >> 8, true);
  transfer(d & 0xFF, true);
}

void GxIO_SPI_RS::startTransaction()
{
  if (_cs >= 0) digitalWrite(_cs, LOW);
}

void GxIO_SPI_RS::endTransaction()
{
  if (_cs >= 0) digitalWrite(_cs, HIGH);
}

void GxIO_SPI_RS::selectRegister(bool rs_low)
{
}

void GxIO_SPI_RS::setBackLight(bool lit)
{
  if (_bl >= 0) digitalWrite(_bl, (lit ? HIGH : LOW));
}

#endif



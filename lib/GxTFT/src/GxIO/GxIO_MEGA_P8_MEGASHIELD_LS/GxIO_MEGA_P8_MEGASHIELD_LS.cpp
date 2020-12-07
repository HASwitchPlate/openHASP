// created by Jean-Marc Zingg to be the GxIO_MEGA_P8_MEGASHIELD_LS io class for the GxTFT library
// code extracts taken from https://github.com/Bodmer/TFT_HX8357
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// for 8 bit parallel displays with latch on shields or on adapter shields for MEGA/DUE, e.g. INHAOS_LCD_2000_9225 on INHAOS MEGA shield

// read functions do not work, even with resistor from pin 43 to RD

#if defined(__AVR_ATmega2560__)

#include "GxIO_MEGA_P8_MEGASHIELD_LS.h"

GxIO_MEGA_P8_MEGASHIELD_LS::GxIO_MEGA_P8_MEGASHIELD_LS(uint8_t latch_strobe_pin)
{
  // INHAOS LCD-2000-9225 on INHAOS MEGA shield
  _cs   = 40; //PORT G bit _BV(1)
  _rs   = 38; //PORT D bit _BV(7)
  _rst  = 41; //PORT G bit _BV(0)
  _wr   = 39; //PORT G bit _BV(2)
  _rd   = 43; //PORT L bit _BV(6)
  _bl   = 44;
  _ls   = latch_strobe_pin;
  _lsport = portOutputRegister(digitalPinToPort(_ls));
  _lsbit  = digitalPinToBitMask(_ls);
}

void GxIO_MEGA_P8_MEGASHIELD_LS::reset()
{
  digitalWrite(_rst, LOW);
  delay(10);
  digitalWrite(_rst, HIGH);
  delay(10);
}

void GxIO_MEGA_P8_MEGASHIELD_LS::init()
{
  digitalWrite(_rst, HIGH);
  digitalWrite(_rs, HIGH);
  digitalWrite(_cs, HIGH);
  digitalWrite(_wr, HIGH);
  //digitalWrite(_rd, HIGH);
  digitalWrite(_rd, LOW); // rd inactive LOW ?
  digitalWrite(_ls, HIGH);
  digitalWrite(_bl, LOW);
  pinMode(_rst, OUTPUT);
  pinMode(_rs, OUTPUT);
  pinMode(_cs, OUTPUT);
  pinMode(_wr, OUTPUT);
  pinMode(_rd, OUTPUT);
  pinMode(_ls, OUTPUT);
  pinMode(_bl, OUTPUT);
  reset();
  DDRC = 0xFF; // Set direction for the 8 bit data port
}

uint8_t GxIO_MEGA_P8_MEGASHIELD_LS::readDataTransaction()
{
#if 1
  DDRC = 0x00; // Set direction input
  PORTG &= ~_BV(1); // CS_L
  PORTL &= ~_BV(6); // RD_L
  PORTL |= _BV(6); // RD_H
  PORTL |= _BV(6); // RD_H
  uint8_t rv = PINC;
  PORTL &= ~_BV(6); // RD_L
  PORTG |= _BV(1); // CS_H
  DDRC = 0xFF; // Set direction output again
  return rv;
#else
  DDRC = 0x00; // Set direction input
  PORTG &= ~_BV(1); // CS_L
  PORTL |= _BV(6); // RD_H
  PORTL &= ~_BV(6); // RD_L
  PORTL &= ~_BV(6); // RD_L
  PORTL &= ~_BV(6); // RD_L
  uint8_t rv = PINC;
  PORTL |= _BV(6); // RD_H
  PORTG |= _BV(1); // CS_H
  DDRC = 0xFF; // Set direction output again
  return rv;
#endif
}

uint16_t GxIO_MEGA_P8_MEGASHIELD_LS::readData16Transaction()
{
  uint16_t rv = readDataTransaction() << 8;
  rv |= readDataTransaction();
  return rv;
}

uint8_t GxIO_MEGA_P8_MEGASHIELD_LS::readData()
{
#if 1
  DDRC = 0x00; // Set direction input
  PORTL &= ~_BV(6); // RD_L
  PORTL |= _BV(6); // RD_H
  PORTL |= _BV(6); // RD_H
  uint8_t rv = PINC;
  PORTL &= ~_BV(6); // RD_L
  DDRC = 0xFF; // Set direction output again
  return rv;
#else
  DDRC = 0x00; // Set direction input
  PORTL |= _BV(6); // RD_H
  PORTL &= ~_BV(6); // RD_L
  PORTL &= ~_BV(6); // RD_L
  PORTL &= ~_BV(6); // RD_L
  uint8_t rv = PINC;
  PORTL |= _BV(6); // RD_H
  DDRC = 0xFF; // Set direction output again
  return rv;
#endif
}

uint16_t GxIO_MEGA_P8_MEGASHIELD_LS::readData16()
{
  uint16_t rv = readData() << 8;
  rv |= readData();
  return rv;
}

void GxIO_MEGA_P8_MEGASHIELD_LS::writeCommandTransaction(uint8_t c)
{
  PORTD &= ~_BV(7); // RS_L
  PORTG &= ~_BV(1); // CS_L
  PORTC = 0;
  *_lsport |= _lsbit; *_lsport &= ~_lsbit; // LS_STB
  PORTC = c;
  PORTG &= ~_BV(2); PORTG |= _BV(2); // WR_STB;
  PORTG |= _BV(1); // CS_H
  PORTD |= _BV(7); // RS_H
}

void GxIO_MEGA_P8_MEGASHIELD_LS::writeDataTransaction(uint8_t d)
{
  PORTG &= ~_BV(1); // CS_L
  PORTC = 0;
  *_lsport |= _lsbit; *_lsport &= ~_lsbit; // LS_STB
  PORTC = d;
  PORTG &= ~_BV(2); PORTG |= _BV(2); // WR_STB;
  PORTG |= _BV(1); // CS_H
}

void GxIO_MEGA_P8_MEGASHIELD_LS::writeData16Transaction(uint16_t d, uint32_t num)
{
  PORTG &= ~_BV(1); // CS_L
  writeData16(d, num);
  PORTG |= _BV(1); // CS_H
}

void GxIO_MEGA_P8_MEGASHIELD_LS::writeCommand(uint8_t c)
{
  PORTD &= ~_BV(7); // RS_L
  PORTG &= ~_BV(1); // CS_L
  PORTC = 0;
  *_lsport |= _lsbit; *_lsport &= ~_lsbit; // LS_STB
  PORTC = c;
  PORTG &= ~_BV(2); PORTG |= _BV(2); // WR_STB;
  PORTG |= _BV(1); // CS_H
  PORTD |= _BV(7); // RS_H
}

void GxIO_MEGA_P8_MEGASHIELD_LS::writeCommand16(uint16_t c)
{
  PORTD &= ~_BV(7); // RS_L
  PORTG &= ~_BV(1); // CS_L
  PORTC = c >> 8;
  *_lsport |= _lsbit; *_lsport &= ~_lsbit; // LS_STB
  PORTC = c;
  PORTG &= ~_BV(2); PORTG |= _BV(2); // WR_STB;
  PORTG |= _BV(1); // CS_H
  PORTD |= _BV(7); // RS_H
}

void GxIO_MEGA_P8_MEGASHIELD_LS::writeData(uint8_t d)
{
  PORTG &= ~_BV(1); // CS_L
  PORTC = 0;
  *_lsport |= _lsbit; *_lsport &= ~_lsbit; // LS_STB
  PORTC = d;
  PORTG &= ~_BV(2); PORTG |= _BV(2); // WR_STB;
  PORTG |= _BV(1); // CS_H
}

void GxIO_MEGA_P8_MEGASHIELD_LS::writeData(uint8_t* d, uint32_t num)
{
  while (num > 0)
  {
    writeData(*d);
    d++;
    num--;
  }
}

void GxIO_MEGA_P8_MEGASHIELD_LS::writeData16(uint16_t d, uint32_t num)
{
  while (num > 0)
  {
    PORTG &= ~_BV(1); // CS_L
    PORTC = d >> 8;
    *_lsport |= _lsbit; *_lsport &= ~_lsbit; // LS_STB
    PORTC = d;
    PORTG &= ~_BV(2); PORTG |= _BV(2); // WR_STB;
    num--;
    PORTG |= _BV(1); // CS_H
  }
}

void GxIO_MEGA_P8_MEGASHIELD_LS::writeAddrMSBfirst(uint16_t d)
{
  writeData16(d);
}

void GxIO_MEGA_P8_MEGASHIELD_LS::startTransaction()
{
  PORTG &= ~_BV(1);
}

void GxIO_MEGA_P8_MEGASHIELD_LS::endTransaction()
{
  PORTG |= _BV(1);
}

void GxIO_MEGA_P8_MEGASHIELD_LS::selectRegister(bool rs_low)
{
  digitalWrite(_rs, (rs_low ? LOW : HIGH));
}

void GxIO_MEGA_P8_MEGASHIELD_LS::setBackLight(bool lit)
{
  if (_bl >= 0) digitalWrite(_bl, (lit ? HIGH : LOW));
}

#endif


// created by Jean-Marc Zingg to be the GxIO_UNO_P8_ROBOTDYN_SHIELD io class for the GxTFT library
// code extracts taken from https://github.com/Bodmer/TFT_HX8357
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// for 8 bit parallel displays on RobotDyn adapter shield for UNO
// e.g. https://www.aliexpress.com/store/product/Expansion-Shield-for-TFT-2-8-LCD-Touch-Screen-for-Uno-Mega/1950989_32711885041.html

#if defined(__AVR_ATmega328P__)

#include "GxIO_UNO_P8_ROBOTDYN_SHIELD.h"

GxIO_UNO_P8_ROBOTDYN_SHIELD::GxIO_UNO_P8_ROBOTDYN_SHIELD() :
#if defined(UCSRB)
  _ucsrb(&UCSRB)
#elif defined(UCSR0B)
  _ucsrb(&UCSR0B)
#else
  _ucsrb(&_dummy_ucsrb)
#endif
{
  _cs   = A3; // PC3
  _rs   = A5; // PC5
  _rst  = A2; // PC2
  _wr   = A4; // PC4
}

void GxIO_UNO_P8_ROBOTDYN_SHIELD::reset()
{
  digitalWrite(_rst, LOW);
  delay(10);
  digitalWrite(_rst, HIGH);
  delay(10);
}

void GxIO_UNO_P8_ROBOTDYN_SHIELD::init()
{
  digitalWrite(_rst, HIGH);
  digitalWrite(_rs, HIGH);
  digitalWrite(_cs, HIGH);
  digitalWrite(_wr, HIGH);
  pinMode(_rst, OUTPUT);
  pinMode(_rs, OUTPUT);
  pinMode(_cs, OUTPUT);
  pinMode(_wr, OUTPUT);
  reset();
  DDRD = 0xFF; // Set direction for the 8 bit data port
}

uint8_t GxIO_UNO_P8_ROBOTDYN_SHIELD::readDataTransaction()
{
  // shield is write-only
  return 0;
}

uint16_t GxIO_UNO_P8_ROBOTDYN_SHIELD::readData16Transaction()
{
  // shield is write-only
  return 0;
}

uint8_t GxIO_UNO_P8_ROBOTDYN_SHIELD::readData()
{
  // shield is write-only
  return 0;
}

uint16_t GxIO_UNO_P8_ROBOTDYN_SHIELD::readData16()
{
  // shield is write-only
  return 0;
}

void GxIO_UNO_P8_ROBOTDYN_SHIELD::writeCommandTransaction(uint8_t c)
{
  uint8_t ucsrb_save = *_ucsrb;
  *_ucsrb = 0;
  PORTC &= ~_BV(3); // CS_L;
  PORTC &= ~_BV(5); // RS_L;
  PORTD = c;
  PORTC &= ~_BV(4); PORTC &= ~_BV(4); PORTC |= _BV(4); // WR_STB;
  PORTC |= _BV(5); // RS_H;
  PORTC |= _BV(3); // CS_H;
  *_ucsrb = ucsrb_save;
}

void GxIO_UNO_P8_ROBOTDYN_SHIELD::writeDataTransaction(uint8_t d)
{
  uint8_t ucsrb_save = *_ucsrb;
  *_ucsrb = 0;
  PORTC &= ~_BV(3); // CS_L;
  PORTD = d;
  PORTC &= ~_BV(4); PORTC &= ~_BV(4); PORTC |= _BV(4); // WR_STB;
  PORTC |= _BV(3); // CS_H;
  *_ucsrb = ucsrb_save;
}

void GxIO_UNO_P8_ROBOTDYN_SHIELD::writeData16Transaction(uint16_t d, uint32_t num)
{
  PORTC &= ~_BV(3); // CS_L;
  writeData16(d, num);
  PORTC |= _BV(3); // CS_H;
}

void GxIO_UNO_P8_ROBOTDYN_SHIELD::writeCommand(uint8_t c)
{
  uint8_t ucsrb_save = *_ucsrb;
  *_ucsrb = 0;
  PORTC &= ~_BV(5); // RS_L;
  PORTD = c;
  PORTC &= ~_BV(4); PORTC &= ~_BV(4); PORTC |= _BV(4); // WR_STB;
  PORTC |= _BV(5); // RS_H;
  *_ucsrb = ucsrb_save;
}

void GxIO_UNO_P8_ROBOTDYN_SHIELD::writeData(uint8_t d)
{
  uint8_t ucsrb_save = *_ucsrb;
  *_ucsrb = 0;
  PORTD = d;
  PORTC &= ~_BV(4); PORTC &= ~_BV(4); PORTC |= _BV(4); // WR_STB;
  *_ucsrb = ucsrb_save;
}

void GxIO_UNO_P8_ROBOTDYN_SHIELD::writeData(uint8_t* d, uint32_t num)
{
  while (num > 0)
  {
    writeData(*d);
    d++;
    num--;
  }
}

void GxIO_UNO_P8_ROBOTDYN_SHIELD::writeData16(uint16_t d, uint32_t num)
{
  uint8_t ucsrb_save = *_ucsrb;
  *_ucsrb = 0;
  while (num > 0)
  {
    PORTD = d >> 8;
    PORTC &= ~_BV(4); PORTC &= ~_BV(4); PORTC |= _BV(4); // WR_STB;
    PORTD = d;
    PORTC &= ~_BV(4); PORTC &= ~_BV(4); PORTC |= _BV(4); // WR_STB;
    num--;
  }
  *_ucsrb = ucsrb_save;
}

void GxIO_UNO_P8_ROBOTDYN_SHIELD::writeAddrMSBfirst(uint16_t d)
{
  uint8_t ucsrb_save = *_ucsrb;
  *_ucsrb = 0;
  PORTD = d >> 8;
  PORTC &= ~_BV(4); PORTC &= ~_BV(4); PORTC |= _BV(4); // WR_STB;
  PORTD = d;
  PORTC &= ~_BV(4); PORTC &= ~_BV(4); PORTC |= _BV(4); // WR_STB;
  *_ucsrb = ucsrb_save;
}

void GxIO_UNO_P8_ROBOTDYN_SHIELD::startTransaction()
{
  PORTC &= ~_BV(3); // CS_L;
}

void GxIO_UNO_P8_ROBOTDYN_SHIELD::endTransaction()
{
  PORTC |= _BV(3); // CS_H;
}

void GxIO_UNO_P8_ROBOTDYN_SHIELD::selectRegister(bool rs_low)
{
  digitalWrite(_rs, (rs_low ? LOW : HIGH));
}

void GxIO_UNO_P8_ROBOTDYN_SHIELD::setBackLight(bool lit)
{
  if (_bl >= 0) digitalWrite(_bl, (lit ? HIGH : LOW));
}

#endif


// created by Jean-Marc Zingg to be the GxIO_MEGA_P16_MEGASHIELD io class for the GxTFT library
// code extracts taken from https://github.com/Bodmer/TFT_HX8357
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// for 16 bit parallel displays on shields or on adapter shields for MEGA/DUE, e.g. HVGA MEGA or MEGA Shield V2.2

// read functions untested, waiting for CTE TFT LCD/SD Shield for MEGA to arrive

#if defined(__AVR_ATmega2560__)

#include "GxIO_MEGA_P16_MEGASHIELD.h"

GxIO_MEGA_P16_MEGASHIELD::GxIO_MEGA_P16_MEGASHIELD()
{
  _cs   = 40; //PORT G bit _BV(1)
  _rs   = 38; //PORT D bit _BV(7)
  _rst  = 41; //PORT G bit _BV(0)
  _wr   = 39; //PORT G bit _BV(2)
  _rd   = 43; //PORT L bit _BV(6)
}

void GxIO_MEGA_P16_MEGASHIELD::reset()
{
  digitalWrite(_rst, LOW);
  delay(10);
  digitalWrite(_rst, HIGH);
  delay(10);
}

void GxIO_MEGA_P16_MEGASHIELD::init()
{
  digitalWrite(_rst, HIGH);
  digitalWrite(_rs, HIGH);
  digitalWrite(_cs, HIGH);
  digitalWrite(_wr, HIGH);
  digitalWrite(_rd, HIGH);
  pinMode(_rst, OUTPUT);
  pinMode(_rs, OUTPUT);
  pinMode(_cs, OUTPUT);
  pinMode(_wr, OUTPUT);
  pinMode(_rd, OUTPUT);
  reset();
  DDRA = 0xFF; // Set direction for the 2 8 bit data ports
  DDRC = 0xFF;
}

uint8_t GxIO_MEGA_P16_MEGASHIELD::readDataTransaction()
{
  DDRA = 0x00; // Set direction input
  DDRC = 0x00;
  PORTG &= ~_BV(1); // CS_L;
  PORTL |= _BV(6); // RD_H
  PORTL &= ~_BV(6); // RD_L
  PORTL &= ~_BV(6); // RD_L
  PORTL &= ~_BV(6); // RD_L
  uint8_t rv = PINC;
  PORTL |= _BV(6); // RD_H
  PORTG |= _BV(1); // CS_H;
  DDRA = 0xFF; // Set direction output again
  DDRC = 0xFF;
  return rv;
}

uint16_t GxIO_MEGA_P16_MEGASHIELD::readData16Transaction()
{
  DDRA = 0x00; // Set direction input
  DDRC = 0x00;
  PORTG &= ~_BV(1); // CS_L;
  PORTL |= _BV(6); // RD_H
  PORTL &= ~_BV(6); // RD_L
  PORTL &= ~_BV(6); // RD_L
  PORTL &= ~_BV(6); // RD_L
  uint16_t rv = (PINA<<8) | (PINC);
  PORTL |= _BV(6); // RD_H
  PORTG |= _BV(1); // CS_H;
  DDRA = 0xFF; // Set direction output again
  DDRC = 0xFF;
  return rv;
}

uint8_t GxIO_MEGA_P16_MEGASHIELD::readData()
{
  DDRA = 0x00; // Set direction input
  DDRC = 0x00;
  PORTL |= _BV(6); // RD_H
  PORTL &= ~_BV(6); // RD_L
  PORTL &= ~_BV(6); // RD_L
  PORTL &= ~_BV(6); // RD_L
  uint8_t rv = PINC;
  PORTL |= _BV(6); // RD_H
  DDRA = 0xFF; // Set direction output again
  DDRC = 0xFF;
  return rv;
}

uint16_t GxIO_MEGA_P16_MEGASHIELD::readData16()
{
  DDRA = 0x00; // Set direction input
  DDRC = 0x00;
  PORTL |= _BV(6); // RD_H
  PORTL &= ~_BV(6); // RD_L
  PORTL &= ~_BV(6); // RD_L
  PORTL &= ~_BV(6); // RD_L
  uint16_t rv = (PINA<<8) | (PINC);
  PORTL |= _BV(6); // RD_H
  DDRA = 0xFF; // Set direction output again
  DDRC = 0xFF;
  return rv;
}

void GxIO_MEGA_P16_MEGASHIELD::writeCommandTransaction(uint8_t c)
{
  PORTG &= ~_BV(1); // CS_L;
  PORTD &= ~_BV(7); // RS_L;
  PORTA = 0;
  PORTC = c;
  PORTG &= ~_BV(2); PORTG &= ~_BV(2); PORTG |= _BV(2); // WR_STB;
  PORTD |= _BV(7); // RS_H;
  PORTG |= _BV(1); // CS_H;
}

void GxIO_MEGA_P16_MEGASHIELD::writeCommand16Transaction(uint16_t c)
{
  PORTG &= ~_BV(1); // CS_L;
  PORTD &= ~_BV(7); // RS_L;
  PORTA = c >> 8;
  PORTC = c;
  PORTG &= ~_BV(2); PORTG &= ~_BV(2); PORTG |= _BV(2); // WR_STB;
  PORTD |= _BV(7); // RS_H;
  PORTG |= _BV(1); // CS_H;
}

void GxIO_MEGA_P16_MEGASHIELD::writeDataTransaction(uint8_t d)
{
  PORTG &= ~_BV(1); // CS_L;
  PORTA = d >> 8;
  PORTC = d;
  PORTG &= ~_BV(2); PORTG &= ~_BV(2); PORTG |= _BV(2); // WR_STB;
  PORTG |= _BV(1); // CS_H;
}

void GxIO_MEGA_P16_MEGASHIELD::writeData16Transaction(uint16_t d, uint32_t num)
{
  PORTG &= ~_BV(1); // CS_L;
  writeData16(d, num);
  PORTG |= _BV(1); // CS_H;
}

void GxIO_MEGA_P16_MEGASHIELD::writeCommand(uint8_t c)
{
  PORTD &= ~_BV(7); // RS_L;
  PORTA = 0;
  PORTC = c;
  PORTG &= ~_BV(2); PORTG &= ~_BV(2); PORTG |= _BV(2); // WR_STB;
  PORTD |= _BV(7); // RS_H;
}

void GxIO_MEGA_P16_MEGASHIELD::writeCommand16(uint16_t c)
{
  PORTD &= ~_BV(7); // RS_L;
  PORTA = c >> 8;
  PORTC = c;
  PORTG &= ~_BV(2); PORTG &= ~_BV(2); PORTG |= _BV(2); // WR_STB;
  PORTD |= _BV(7); // RS_H;
}

void GxIO_MEGA_P16_MEGASHIELD::writeData(uint8_t d)
{
  PORTA = d >> 8;
  PORTC = d;
  PORTG &= ~_BV(2); PORTG &= ~_BV(2); PORTG |= _BV(2); // WR_STB;
}

void GxIO_MEGA_P16_MEGASHIELD::writeData(uint8_t* d, uint32_t num)
{
  while (num > 0)
  {
    writeData(*d);
    d++;
    num--;
  }
}

void GxIO_MEGA_P16_MEGASHIELD::writeData16(uint16_t d, uint32_t num)
{
  PORTA = d >> 8;
  PORTC = d;
  while (num > 0)
  {
    PORTG &= ~_BV(2); PORTG &= ~_BV(2); PORTG |= _BV(2);
    num--;
  }
}

void GxIO_MEGA_P16_MEGASHIELD::writeAddrMSBfirst(uint16_t d)
{
  PORTC = d >> 8;
  PORTG &= ~_BV(2); PORTG &= ~_BV(2); PORTG |= _BV(2); // WR_STB;
  PORTC = d;
  PORTG &= ~_BV(2); PORTG &= ~_BV(2); PORTG |= _BV(2); // WR_STB;
}

void GxIO_MEGA_P16_MEGASHIELD::startTransaction()
{
  PORTG &= ~_BV(1);
}

void GxIO_MEGA_P16_MEGASHIELD::endTransaction()
{
  PORTG |= _BV(1);
}

void GxIO_MEGA_P16_MEGASHIELD::selectRegister(bool rs_low)
{
  digitalWrite(_rs, (rs_low ? LOW : HIGH));
}

void GxIO_MEGA_P16_MEGASHIELD::setBackLight(bool lit)
{
  if (_bl >= 0) digitalWrite(_bl, (lit ? HIGH : LOW));
}

#endif

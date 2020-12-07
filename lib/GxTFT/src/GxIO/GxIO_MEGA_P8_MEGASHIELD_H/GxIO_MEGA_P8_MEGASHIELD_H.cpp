// created by Jean-Marc Zingg to be the GxIO_MEGA_P8_MEGASHIELD_H io class for the GxTFT library
// code extracts taken from https://github.com/Bodmer/TFT_HX8357
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// for 8 bit parallel displays on shields or on adapter shields for MEGA, using PORTA to high byte

// read functions work on my modified INHAOS MEGA shield (remove R7, add 1k from there to pin 43)
// I had to add 3k9 pull-up resistors to the data lines on the MEGA side of the series resistors

#if defined(__AVR_ATmega2560__)

#include "GxIO_MEGA_P8_MEGASHIELD_H.h"

GxIO_MEGA_P8_MEGASHIELD_H::GxIO_MEGA_P8_MEGASHIELD_H()
{
  _cs   = 40; //PORT G bit _BV(1)
  _rs   = 38; //PORT D bit _BV(7)
  _rst  = 41; //PORT G bit _BV(0)
  _wr   = 39; //PORT G bit _BV(2)
  _rd   = 43; //PORT L bit _BV(6)
  _bl   = -1;
}

void GxIO_MEGA_P8_MEGASHIELD_H::reset()
{
  digitalWrite(_rst, LOW);
  delay(10);
  digitalWrite(_rst, HIGH);
  delay(10);
}

void GxIO_MEGA_P8_MEGASHIELD_H::init()
{
  digitalWrite(_rd, HIGH);
  pinMode(_rd, OUTPUT);
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
  DDRA = 0xFF; // Set direction for the 8 bit data port
}

uint8_t GxIO_MEGA_P8_MEGASHIELD_H::readDataTransaction()
{
  DDRA = 0x00; // Set direction input
  PORTG &= ~_BV(1); // CS_L;
  PORTL &= ~_BV(6); // RD_L
  PORTL &= ~_BV(6); // RD_L
  uint8_t rv = PINA;
  PORTL |= _BV(6); // RD_H
  PORTG |= _BV(1); // CS_H;
  DDRA = 0xFF; // Set direction output again
  return rv;
}

uint16_t GxIO_MEGA_P8_MEGASHIELD_H::readData16Transaction()
{
  PORTG &= ~_BV(1); // CS_L;
  uint16_t rv = readData16();
  PORTG |= _BV(1); // CS_H;
  return rv;
}

uint8_t GxIO_MEGA_P8_MEGASHIELD_H::readData()
{
  DDRA = 0x00; // Set direction input
  PORTL &= ~_BV(6); // RD_L
  PORTL &= ~_BV(6); // RD_L
  uint8_t rv = PINA;
  PORTL |= _BV(6); // RD_H
  DDRA = 0xFF; // Set direction output again
  return rv;
}

uint16_t GxIO_MEGA_P8_MEGASHIELD_H::readData16()
{
  DDRA = 0x00; // Set direction input
  PORTL &= ~_BV(6); // RD_L
  PORTL &= ~_BV(6); // RD_L
  uint16_t rv = PINA << 8;
  PORTL |= _BV(6); // RD_H
  PORTL &= ~_BV(6); // RD_L
  PORTL &= ~_BV(6); // RD_L
  rv |= PINA;
  PORTL |= _BV(6); // RD_H
  DDRA = 0xFF; // Set direction output again
  return rv;
}

void GxIO_MEGA_P8_MEGASHIELD_H::writeCommandTransaction(uint8_t c)
{
  PORTG &= ~_BV(1); // CS_L;
  PORTD &= ~_BV(7); // RS_L
  PORTA = c;
  PORTG &= ~_BV(2); PORTG &= ~_BV(2); PORTG |= _BV(2); // WR_STB
  PORTD |= _BV(7); // RS_H;
  PORTG |= _BV(1); // CS_H;
}

void GxIO_MEGA_P8_MEGASHIELD_H::writeDataTransaction(uint8_t d)
{
  PORTG &= ~_BV(1); // CS_L;
  PORTA = d;
  PORTG &= ~_BV(2); PORTG &= ~_BV(2); PORTG |= _BV(2); // WR_STB
  PORTG |= _BV(1); // CS_H;
}

void GxIO_MEGA_P8_MEGASHIELD_H::writeData16Transaction(uint16_t d, uint32_t num)
{
  PORTG &= ~_BV(1); // CS_L;
  writeData16(d, num);
  PORTG |= _BV(1); // CS_H;
}

void GxIO_MEGA_P8_MEGASHIELD_H::writeCommand(uint8_t c)
{
  PORTD &= ~_BV(7); // RS_L
  PORTA = c;
  PORTG &= ~_BV(2); PORTG &= ~_BV(2); PORTG |= _BV(2); // WR_STB
  PORTD |= _BV(7); // RS_H;
}

void GxIO_MEGA_P8_MEGASHIELD_H::writeData(uint8_t d)
{
  PORTA = d;
  PORTG &= ~_BV(2); PORTG &= ~_BV(2); PORTG |= _BV(2); // WR_STB
}

void GxIO_MEGA_P8_MEGASHIELD_H::writeData(uint8_t* d, uint32_t num)
{
  while (num > 0)
  {
    writeData(*d);
    d++;
    num--;
  }
}

void GxIO_MEGA_P8_MEGASHIELD_H::writeData16(uint16_t d, uint32_t num)
{
  while (num > 0)
  {
    writeData(d >> 8);
    writeData(d);
    num--;
  }
}

void GxIO_MEGA_P8_MEGASHIELD_H::writeAddrMSBfirst(uint16_t d)
{
  PORTA = d >> 8;
  PORTG &= ~_BV(2); PORTG &= ~_BV(2); PORTG |= _BV(2); // WR_STB
  PORTA = d;
  PORTG &= ~_BV(2); PORTG &= ~_BV(2); PORTG |= _BV(2); // WR_STB
}

void GxIO_MEGA_P8_MEGASHIELD_H::startTransaction()
{
  PORTG &= ~_BV(1);
}

void GxIO_MEGA_P8_MEGASHIELD_H::endTransaction()
{
  PORTG |= _BV(1);
}

void GxIO_MEGA_P8_MEGASHIELD_H::selectRegister(bool rs_low)
{
  digitalWrite(_rs, (rs_low ? LOW : HIGH));
}

void GxIO_MEGA_P8_MEGASHIELD_H::setBackLight(bool lit)
{
  if (_bl >= 0) digitalWrite(_bl, (lit ? HIGH : LOW));
}

#endif


// created by Jean-Marc Zingg to be the GxIO_UNO_P8_SHIELD io class for the GxTFT library
// code extracts taken from https://github.com/Bodmer/TFT_HX8357
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// for 8 bit parallel displays on shields or on adapter shields for UNO, e.g. mcufriend 8bit tft shields

#if defined(__AVR_ATmega328P__)

#include "GxIO_UNO_P8_SHIELD.h"

#define BMASK         0x03
#define DMASK         0xFC

GxIO_UNO_P8_SHIELD::GxIO_UNO_P8_SHIELD()
{
  _cs   = A3; // PC3
  _rs   = A2; // PC2
  _rst  = A4; // PC4
  _wr   = A1; // PC1
  _rd   = A0; // PC0
}

void GxIO_UNO_P8_SHIELD::reset()
{
  digitalWrite(_rst, LOW);
  delay(10);
  digitalWrite(_rst, HIGH);
  delay(10);
}

void GxIO_UNO_P8_SHIELD::init()
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
  DDRB = BMASK; // Set direction for the 2 8 bit data ports
  DDRD = DMASK;
}

uint8_t GxIO_UNO_P8_SHIELD::readDataTransaction()
{
  PORTC &= ~_BV(3); // CS_L;
  uint8_t rv = readData();
  PORTC |= _BV(3); // CS_H;
  return rv;
}

uint16_t GxIO_UNO_P8_SHIELD::readData16Transaction()
{
  PORTC &= ~_BV(3); // CS_L;
  uint16_t rv = readData16();
  PORTC |= _BV(3); // CS_H;
  return rv;
}

uint8_t GxIO_UNO_P8_SHIELD::readData()
{
  DDRB &= ~BMASK; // Set direction input
  DDRD &= ~DMASK;
  PORTC &= ~_BV(0); // RD_L; // no pre-strobe
  PORTC &= ~_BV(0); // RD_L; // delay read
  uint8_t rv = ( (PINB & BMASK) | (PIND & DMASK) );
  PORTC |= _BV(0); // RD_H;
  DDRB |= BMASK; // Set direction output again
  DDRD |= DMASK;
  return rv;
}

uint16_t GxIO_UNO_P8_SHIELD::readData16()
{
  uint16_t rv = readData() << 8;
  rv |= readData();
  return rv;
}

void GxIO_UNO_P8_SHIELD::writeCommandTransaction(uint8_t c)
{
  PORTC &= ~_BV(3); // CS_L;
  PORTC &= ~_BV(2); // RS_L;
  PORTB = (PORTB & ~BMASK) | ((c) & BMASK);
  PORTD = (PORTD & ~DMASK) | ((c) & DMASK);
  PORTC &= ~_BV(1); PORTC |= _BV(1); // WR_STB;
  PORTC |= _BV(2); // RS_H;
  PORTC |= _BV(3); // CS_H;
}

void GxIO_UNO_P8_SHIELD::writeDataTransaction(uint8_t d)
{
  PORTC &= ~_BV(3); // CS_L;
  PORTB = (PORTB & ~BMASK) | ((d) & BMASK);
  PORTD = (PORTD & ~DMASK) | ((d) & DMASK);
  PORTC &= ~_BV(1); PORTC |= _BV(1); // WR_STB;
  PORTC |= _BV(3); // CS_H;
}

void GxIO_UNO_P8_SHIELD::writeData16Transaction(uint16_t d, uint32_t num)
{
  PORTC &= ~_BV(3); // CS_L;
  writeData16(d, num);
  PORTC |= _BV(3); // CS_H;
}

void GxIO_UNO_P8_SHIELD::writeCommand(uint8_t c)
{
  PORTC &= ~_BV(2); // RS_L;
  PORTB = (PORTB & ~BMASK) | ((c) & BMASK);
  PORTD = (PORTD & ~DMASK) | ((c) & DMASK);
  PORTC &= ~_BV(1); PORTC |= _BV(1); // WR_STB;
  PORTC |= _BV(2); // RS_H;
}

void GxIO_UNO_P8_SHIELD::writeData(uint8_t d)
{
  PORTB = (PORTB & ~BMASK) | ((d) & BMASK);
  PORTD = (PORTD & ~DMASK) | ((d) & DMASK);
  PORTC &= ~_BV(1); PORTC |= _BV(1); // WR_STB;
}

void GxIO_UNO_P8_SHIELD::writeData(uint8_t* d, uint32_t num)
{
  while (num > 0)
  {
    writeData(*d);
    d++;
    num--;
  }
}

void GxIO_UNO_P8_SHIELD::writeData16(uint16_t d, uint32_t num)
{
  while (num > 0)
  {
    PORTB = (PORTB & ~BMASK) | ((d >> 8) & BMASK);
    PORTD = (PORTD & ~DMASK) | ((d >> 8) & DMASK);
    PORTC &= ~_BV(1); PORTC |= _BV(1); // WR_STB;
    PORTB = (PORTB & ~BMASK) | ((d) & BMASK);
    PORTD = (PORTD & ~DMASK) | ((d) & DMASK);
    PORTC &= ~_BV(1); PORTC |= _BV(1); // WR_STB;
    num--;
  }
}

void GxIO_UNO_P8_SHIELD::writeAddrMSBfirst(uint16_t d)
{
  PORTB = (PORTB & ~BMASK) | ((d >> 8) & BMASK);
  PORTD = (PORTD & ~DMASK) | ((d >> 8) & DMASK);
  PORTC &= ~_BV(1); PORTC |= _BV(1); // WR_STB;
  PORTB = (PORTB & ~BMASK) | ((d) & BMASK);
  PORTD = (PORTD & ~DMASK) | ((d) & DMASK);
  PORTC &= ~_BV(1); PORTC |= _BV(1); // WR_STB;
}

void GxIO_UNO_P8_SHIELD::startTransaction()
{
  PORTC &= ~_BV(3); // CS_L;
}

void GxIO_UNO_P8_SHIELD::endTransaction()
{
  PORTC |= _BV(3); // CS_H;
}

void GxIO_UNO_P8_SHIELD::selectRegister(bool rs_low)
{
  digitalWrite(_rs, (rs_low ? LOW : HIGH));
}

void GxIO_UNO_P8_SHIELD::setBackLight(bool lit)
{
  if (_bl >= 0) digitalWrite(_bl, (lit ? HIGH : LOW));
}

#endif


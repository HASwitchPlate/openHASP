// created by Jean-Marc Zingg to be the GxIO_DUE_P16_TIKY io class for the GxTFT library
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// this is a special wiring for
// https://www.aliexpress.com/item/5-0-inch-HD-IPS-TFT-LCD-module-resistance-touch-with-PCB-adapter-board-854-480/32666829945.html
//
// for pin mapping see http://forum.arduino.cc/index.php?topic=366304.msg3190917#msg3190917
//

#if defined(ARDUINO_ARCH_SAM)

#include "GxIO_DUE_P16_TIKY.h"

GxIO_DUE_P16_TIKY::GxIO_DUE_P16_TIKY()
{
  _cs   = 22; // PB26
  _rs   = 23; // PA14
  _rst  = 26; // PD1
  _wr   = 24; // PA15
  _rd   = 25; // PD0
  _bl   = 49; // PC14
}

void GxIO_DUE_P16_TIKY::reset()
{
  digitalWrite(_rst, LOW);
  delay(10);
  digitalWrite(_rst, HIGH);
  delay(10);
}

void GxIO_DUE_P16_TIKY::init()
{
  digitalWrite(_rst, HIGH);
  digitalWrite(_rs, HIGH);
  digitalWrite(_cs, HIGH);
  digitalWrite(_wr, HIGH);
  digitalWrite(_rd, HIGH);
  digitalWrite(_bl, LOW);
  pinMode(_rst, OUTPUT);
  pinMode(_rs, OUTPUT);
  pinMode(_cs, OUTPUT);
  pinMode(_wr, OUTPUT);
  pinMode(_rd, OUTPUT);
  pinMode(_bl, OUTPUT);
  reset();
  // Set 16 bit port pins to output on Due
  setDataPins(OUTPUT);
}

void GxIO_DUE_P16_TIKY::setDataPins(uint8_t mode)
{
  for (uint8_t pin = 27; pin <= 42; pin++)
  {
    pinMode(pin, mode);
  }
}

uint8_t GxIO_DUE_P16_TIKY::readDataTransaction()
{
  return readData16Transaction();
}

uint16_t GxIO_DUE_P16_TIKY::readData16Transaction()
{
  REG_PIOB_CODR = 0x1 << 26; // PB26 CS_L
  uint16_t rv = readData16();
  REG_PIOB_SODR = 0x1 << 26; // PB26 CS_H
  return rv;
}

uint8_t GxIO_DUE_P16_TIKY::readData()
{
  return readData16();
}

uint16_t GxIO_DUE_P16_TIKY::readData16()
{
  setDataPins(INPUT);
  //REG_PIOD_CODR = 0x1; // PD0 RD_L pulse
  //REG_PIOD_SODR = 0x1; // PD0 RD_H
  REG_PIOD_CODR = 0x1; // PD0 RD_L read
  uint32_t rv = 0;
  // The compiler efficiently codes this
  // so it is quite quick.                    Port.bit
  rv |= (REG_PIOA_PDSR & (0x1 << 19)) >> -(15 - 19); // A19
  rv |= (REG_PIOC_PDSR & (0x1 << 9)) << (14 - 9); // C9
  rv |= (REG_PIOC_PDSR & (0x1 << 8)) << (13 - 8); // C8
  rv |= (REG_PIOC_PDSR & (0x1 << 7)) << (12 - 7); // C7
  rv |= (REG_PIOC_PDSR & (0x1 << 6)) << (11 - 6); // C6
  rv |= (REG_PIOC_PDSR & (0x1 << 5)) << (10 - 5); // C5
  rv |= (REG_PIOC_PDSR & (0x1 << 4)) << (9 - 4); // C4
  rv |= (REG_PIOC_PDSR & (0x1 << 3)) << (8 - 3); // C3
  // so it is quite quick.                    Port.bit
  rv |= (REG_PIOC_PDSR & (0x1 << 2)) << (7 - 2); // C2
  rv |= (REG_PIOC_PDSR & (0x1 << 1)) << (6 - 1); // C1
  rv |= (REG_PIOD_PDSR & (0x1 << 10)) >> -(5 - 10); // D10
  rv |= (REG_PIOA_PDSR & (0x1 << 7)) >> -(4 - 7); // A7
  rv |= (REG_PIOD_PDSR & (0x1 << 9)) >> -(3 - 9); // D9
  rv |= (REG_PIOD_PDSR & (0x1 << 6)) >> -(2 - 6); // D6
  rv |= (REG_PIOD_PDSR & (0x1 << 3)) >> -(1 - 3); // D3
  rv |= (REG_PIOD_PDSR & (0x1 << 2)) >> -(0 - 2); // D2
  REG_PIOD_SODR = 0x1; // PD0 RD_H
  setDataPins(OUTPUT);
  return rv;
}

uint32_t GxIO_DUE_P16_TIKY::readRawData32(uint8_t part)
{
  setDataPins(INPUT);
  REG_PIOD_CODR = 0x1; // PD0 RD_L pulse
  REG_PIOD_SODR = 0x1; // PD0 RD_H
  REG_PIOD_CODR = 0x1; // PD0 RD_L read
  uint32_t rv = 0;
  if (part == 0) rv = REG_PIOA_PDSR;
  if (part == 1) rv = REG_PIOC_PDSR;
  if (part == 2) rv = REG_PIOD_PDSR;
  REG_PIOD_SODR = 0x1; // PD0 RD_H
  setDataPins(OUTPUT);
  return rv;
}

void GxIO_DUE_P16_TIKY::writeCommandTransaction(uint8_t c)
{
  REG_PIOB_CODR = 0x1 << 26; // PB26 CS_L
  REG_PIOA_CODR = 0x1 << 14; // PA14 RS_L
  writeData16(c);
  REG_PIOA_SODR = 0x1 << 14; // PA14 RS_H
  REG_PIOB_SODR = 0x1 << 26; // PB26 CS_H
}

void GxIO_DUE_P16_TIKY::writeCommand16Transaction(uint16_t c)
{
  REG_PIOB_CODR = 0x1 << 26; // PB26 CS_L
  REG_PIOA_CODR = 0x1 << 14; // PA14 RS_L
  writeData16(c);
  REG_PIOA_SODR = 0x1 << 14; // PA14 RS_H
  REG_PIOB_SODR = 0x1 << 26; // PB26 CS_H
}

void GxIO_DUE_P16_TIKY::writeDataTransaction(uint8_t d)
{
  REG_PIOB_CODR = 0x1 << 26; // PB26 CS_L
  writeData16(d);
  REG_PIOB_SODR = 0x1 << 26; // PB26 CS_H
}

void GxIO_DUE_P16_TIKY::writeData16Transaction(uint16_t d, uint32_t num)
{
  REG_PIOB_CODR = 0x1 << 26; // PB26 CS_L
  writeData16(d, num);
  REG_PIOB_SODR = 0x1 << 26; // PB26 CS_H
}

void GxIO_DUE_P16_TIKY::writeCommand(uint8_t c)
{
  REG_PIOA_CODR = 0x1 << 14; // PA14 RS_L
  writeData16(c);
  REG_PIOA_SODR = 0x1 << 14; // PA14 RS_H
}

void GxIO_DUE_P16_TIKY::writeCommand16(uint16_t c)
{
  REG_PIOA_CODR = 0x1 << 14; // PA14 RS_L
  writeData16(c);
  REG_PIOA_SODR = 0x1 << 14; // PA14 RS_H
}

void GxIO_DUE_P16_TIKY::writeData(uint8_t d)
{
  writeData16(d);
}

void GxIO_DUE_P16_TIKY::writeData(uint8_t* d, uint32_t num)
{
  while (num > 0)
  {
    writeData16(*d);
    d++;
    num--;
  }
}

void GxIO_DUE_P16_TIKY::writeData16(uint16_t d, uint32_t num)
{
  //                |       |       |       |       ; // Ruler for byte MS bits 31, 23, 15 and 7
  REG_PIOA_CODR = 0b00000000000110000000000010000000; // Clear bits A7,A19,A20
  //                |       |       |       |       ;
  REG_PIOB_CODR = 0b00000000000000000000000000000000; // Clear bits
  //                |       |       |       |       ;
  REG_PIOC_CODR = 0b00000000000000000000001111111110; // Clear bits C1..C9
  //                |       |       |       |       ;
  REG_PIOD_CODR = 0b00000000000000000000011001001100; // Clear bits D2,D3,D6,D9,D10
  //                |       |       |       |
  // The compiler efficiently codes this
  // so it is quite quick.                    Port.bit
  if (d & 0x8000) REG_PIOA_SODR = 0x1 << 19; // A19
  if (d & 0x4000) REG_PIOC_SODR = 0x1 << 9; // C9
  if (d & 0x2000) REG_PIOC_SODR = 0x1 << 8; // C8
  if (d & 0x1000) REG_PIOC_SODR = 0x1 << 7; // C7
  if (d & 0x0800) REG_PIOC_SODR = 0x1 << 6; // C6
  if (d & 0x0400) REG_PIOC_SODR = 0x1 << 5; // C5
  if (d & 0x0200) REG_PIOC_SODR = 0x1 << 4; // C4
  if (d & 0x0100) REG_PIOC_SODR = 0x1 << 3; // C3

  // so it is quite quick.                    Port.bit
  if (d & 0x0080) REG_PIOC_SODR = 0x1 << 2; // C2
  if (d & 0x0040) REG_PIOC_SODR = 0x1 << 1; // C1
  if (d & 0x0020) REG_PIOD_SODR = 0x1 << 10; // D10
  if (d & 0x0010) REG_PIOA_SODR = 0x1 << 7; // A7
  if (d & 0x0008) REG_PIOD_SODR = 0x1 << 9; // D9
  if (d & 0x0004) REG_PIOD_SODR = 0x1 << 6; // D6
  if (d & 0x0002) REG_PIOD_SODR = 0x1 << 3; // D3
  if (d & 0x0001) REG_PIOD_SODR = 0x1 << 2; // D2
  while (num > 0)
  {
    // WR_STB;
    REG_PIOA_CODR = 0x1 << 15;  
    REG_PIOA_CODR = 0x1 << 15;  
    REG_PIOA_CODR = 0x1 << 15;  
    REG_PIOA_SODR = 0x1 << 15;
    num--;
  }
}

void GxIO_DUE_P16_TIKY::writeAddrMSBfirst(uint16_t d)
{
  writeData16(d >> 8);
  writeData16(d & 0xFF);
}

void GxIO_DUE_P16_TIKY::startTransaction()
{
  REG_PIOB_CODR = 0x1 << 26; // PB26 CS_L
}

void GxIO_DUE_P16_TIKY::endTransaction()
{
  REG_PIOB_SODR = 0x1 << 26; // PB26 CS_H
}

void GxIO_DUE_P16_TIKY::selectRegister(bool rs_low)
{
  digitalWrite(_rs, (rs_low ? LOW : HIGH));
}

void GxIO_DUE_P16_TIKY::setBackLight(bool lit)
{
  digitalWrite(_bl, (lit ? HIGH : LOW));
}

#endif

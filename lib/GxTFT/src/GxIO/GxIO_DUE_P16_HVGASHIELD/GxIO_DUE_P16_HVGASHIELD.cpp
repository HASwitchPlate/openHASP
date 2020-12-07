// created by Jean-Marc Zingg to be the GxIO_DUE_P16_HVGASHIELD io class for the GxTFT library
// code extracts taken from https://github.com/Bodmer/TFT_HX8357_Due
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// for 16 bit parallel displays on shields for MEGA, usually marked HVGA 480X320 3.2" TFTLCD Shield for Arduino Mega2560
// this is an example how to achieve an arbitrary pin mapping in software for Arduino Due.
//
// for optimal performance use Bodmer's TFT_HX8357_Due library instead.

#if defined(ARDUINO_ARCH_SAM)

#include "GxIO_DUE_P16_HVGASHIELD.h"

GxIO_DUE_P16_HVGASHIELD::GxIO_DUE_P16_HVGASHIELD()
{
  _cs   = 40;
  _rs   = 38;
  _rst  = 41;
  _wr   = 39;
  _rd   = 43;
}

void GxIO_DUE_P16_HVGASHIELD::reset()
{
  digitalWrite(_rst, LOW);
  delay(10);
  digitalWrite(_rst, HIGH);
  delay(10);
}

void GxIO_DUE_P16_HVGASHIELD::init()
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
  // Set 16 bit port pins to output on Due
  setDataPins(OUTPUT);
}

void GxIO_DUE_P16_HVGASHIELD::setDataPins(uint8_t mode)
{
  pinMode(29, mode);
  pinMode(28, mode);
  pinMode(27, mode);
  pinMode(26, mode);
  pinMode(25, mode);
  pinMode(24, mode);
  pinMode(23, mode);
  pinMode(22, mode);
  pinMode(30, mode);
  pinMode(31, mode);
  pinMode(32, mode);
  pinMode(33, mode);
  pinMode(34, mode);
  pinMode(35, mode);
  pinMode(36, mode);
  pinMode(37, mode);
}

uint8_t GxIO_DUE_P16_HVGASHIELD::readDataTransaction()
{
  return readData16Transaction();
}

uint16_t GxIO_DUE_P16_HVGASHIELD::readData16Transaction()
{
  digitalWrite(_cs, LOW);
  uint16_t rv = readData16();
  digitalWrite(_cs, HIGH);
  return rv;
}

uint8_t GxIO_DUE_P16_HVGASHIELD::readData()
{
  return readData16();
}

uint16_t GxIO_DUE_P16_HVGASHIELD::readData16()
{
  setDataPins(INPUT);
  digitalWrite(_rd, LOW);
  digitalWrite(_rd, HIGH);
  digitalWrite(_rd, HIGH);
  digitalWrite(_rd, LOW);
  digitalWrite(_rd, LOW);
  uint32_t rv = 0;
  // The compiler efficiently codes this
  // so it is quite quick.                           Port.bit
  rv |= (REG_PIOD_PDSR & (0x1 << 6)) << (15 - 6); // D.6
  rv |= (REG_PIOD_PDSR & (0x1 << 3)) << (14 - 3); // D.3
  rv |= (REG_PIOD_PDSR & (0x1 << 2)) << (13 - 2); // D.2
  rv |= (REG_PIOD_PDSR & (0x1 << 1)) << (12 - 1); // D.1
  rv |= (REG_PIOD_PDSR & (0x1 << 0)) << (11 - 0); // D.0
  rv |= (REG_PIOA_PDSR & (0x1 << 15)) >> -(10 - 15); // A.15
  rv |= (REG_PIOA_PDSR & (0x1 << 14)) >> -(9 - 14); // A.14
  rv |= (REG_PIOB_PDSR & (0x1 << 26)) >> -(8 - 26); // B.26
  // so it is quite quick.                           Port.bit
  rv |= (REG_PIOD_PDSR & (0x1 << 9)) >> -(7 - 9); // D.9
  rv |= (REG_PIOA_PDSR & (0x1 << 7)) >> -(6 - 7); // A.7
  rv |= (REG_PIOD_PDSR & (0x1 << 10)) >> -(5 - 10); // D.10
  rv |= (REG_PIOC_PDSR & (0x1 << 1)) << (4 - 1); // C.1
  rv |= (REG_PIOC_PDSR & (0x1 << 2)) << (3 - 2); // C.2
  rv |= (REG_PIOC_PDSR & (0x1 << 3)) >> -(2 - 3); // C.3
  rv |= (REG_PIOC_PDSR & (0x1 << 4)) >> -(1 - 4); // C.4
  rv |= (REG_PIOC_PDSR & (0x1 << 5)) >> -(0 - 5); // C.5
  digitalWrite(_rd, HIGH);
  setDataPins(OUTPUT);
  return rv;
}

uint32_t GxIO_DUE_P16_HVGASHIELD::readRawData32(uint8_t part)
{
  setDataPins(INPUT);
  digitalWrite(_rd, LOW);
  digitalWrite(_rd, HIGH);
  digitalWrite(_rd, HIGH);
  digitalWrite(_rd, LOW);
  digitalWrite(_rd, LOW);
  uint32_t rv = 0;
  if (part == 0) rv = REG_PIOA_PDSR;
  if (part == 1) rv = REG_PIOC_PDSR;
  if (part == 2) rv = REG_PIOD_PDSR;
  digitalWrite(_rd, HIGH);
  setDataPins(OUTPUT);
  return rv;
}

void GxIO_DUE_P16_HVGASHIELD::writeCommandTransaction(uint8_t c)
{
  digitalWrite(_cs, LOW);
  digitalWrite(_rs, LOW);
  writeData16(c);
  digitalWrite(_rs, HIGH);
  digitalWrite(_cs, HIGH);
}

void GxIO_DUE_P16_HVGASHIELD::writeCommand16Transaction(uint16_t c)
{
  digitalWrite(_cs, LOW);
  digitalWrite(_rs, LOW);
  writeData16(c);
  digitalWrite(_rs, HIGH);
  digitalWrite(_cs, HIGH);
}

void GxIO_DUE_P16_HVGASHIELD::writeDataTransaction(uint8_t d)
{
  digitalWrite(_cs, LOW);
  writeData16(d);
  digitalWrite(_cs, HIGH);
}

void GxIO_DUE_P16_HVGASHIELD::writeData16Transaction(uint16_t d, uint32_t num)
{
  digitalWrite(_cs, LOW);
  writeData16(d, num);
  digitalWrite(_cs, HIGH);
}

void GxIO_DUE_P16_HVGASHIELD::writeCommand(uint8_t c)
{
  digitalWrite(_rs, LOW);
  writeData16(c);
  digitalWrite(_rs, HIGH);
}

void GxIO_DUE_P16_HVGASHIELD::writeCommand16(uint16_t c)
{
  digitalWrite(_rs, LOW);
  writeData16(c);
  digitalWrite(_rs, HIGH);
}

void GxIO_DUE_P16_HVGASHIELD::writeData(uint8_t d)
{
  writeData16(d);
}

void GxIO_DUE_P16_HVGASHIELD::writeData(uint8_t* d, uint32_t num)
{
  while (num > 0)
  {
    writeData16(*d);
    d++;
    num--;
  }
}

void GxIO_DUE_P16_HVGASHIELD::writeData16(uint16_t d, uint32_t num)
{
  //                |       |       |       |         Ruler for byte MS bits 31, 23, 15 and 7
  //                     B          AA   DD AD  DDDD  Marker for register bits used
  REG_PIOA_CODR = 0b00000000000000001100000010000000; // Clear bits
  REG_PIOB_CODR = 0b00000100000000000000000000000000; // Clear bits
  //                                        W CCCCC   // WR bit
  REG_PIOC_CODR = 0b00000000000000000000000010111110; // Clear WR bit as well
  REG_PIOD_CODR = 0b00000000000000000000011001001111; // Clear bits

  // The compiler efficiently codes this
  // so it is quite quick.                    Port.bit
  if (d & 0x8000) REG_PIOD_SODR = 0x1 << 6; // D.6
  if (d & 0x4000) REG_PIOD_SODR = 0x1 << 3; // D.3
  if (d & 0x2000) REG_PIOD_SODR = 0x1 << 2; // D.2
  if (d & 0x1000) REG_PIOD_SODR = 0x1 << 1; // D.1
  if (d & 0x0800) REG_PIOD_SODR = 0x1 << 0; // D.0
  if (d & 0x0400) REG_PIOA_SODR = 0x1 << 15; // A.15
  if (d & 0x0200) REG_PIOA_SODR = 0x1 << 14; // A.14
  if (d & 0x0100) REG_PIOB_SODR = 0x1 << 26; // B.26

  // so it is quite quick.                    Port.bit
  if (d & 0x0080) REG_PIOD_SODR = 0x1 << 9; // D.9
  if (d & 0x0040) REG_PIOA_SODR = 0x1 << 7; // A.7
  if (d & 0x0020) REG_PIOD_SODR = 0x1 << 10; // D.10
  if (d & 0x0010) REG_PIOC_SODR = 0x1 << 1; // C.1
  if (d & 0x0008) REG_PIOC_SODR = 0x1 << 2; // C.2
  if (d & 0x0004) REG_PIOC_SODR = 0x1 << 3; // C.3
  if (d & 0x0002) REG_PIOC_SODR = 0x1 << 4; // C.4
  if (d & 0x0001) REG_PIOC_SODR = 0x1 << 5; // C.5
  while (num > 0)
  {
    // WR_STB;
    REG_PIOC_CODR = 0x1 << 7;  
    REG_PIOC_CODR = 0x1 << 7;  
    REG_PIOC_CODR = 0x1 << 7;  
    REG_PIOC_SODR = 0x1 << 7;
    num--;
  }
}

void GxIO_DUE_P16_HVGASHIELD::writeAddrMSBfirst(uint16_t d)
{
  writeData16(d >> 8);
  writeData16(d & 0xFF);
}

void GxIO_DUE_P16_HVGASHIELD::startTransaction()
{
  digitalWrite(_cs, LOW);
}

void GxIO_DUE_P16_HVGASHIELD::endTransaction()
{
  digitalWrite(_cs, HIGH);
}

void GxIO_DUE_P16_HVGASHIELD::selectRegister(bool rs_low)
{
  digitalWrite(_rs, (rs_low ? LOW : HIGH));
}

void GxIO_DUE_P16_HVGASHIELD::setBackLight(bool lit)
{
  if (_bl >= 0) digitalWrite(_bl, (lit ? HIGH : LOW));
}

#endif

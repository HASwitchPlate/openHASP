// created by Jean-Marc Zingg to be the GxIO_DUE_P16_R_SHIELD io class for the GxTFT library
// code extracts taken from https://github.com/Bodmer/TFT_HX8357_Due
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// this is a variant of GxIO_DUE_P16_DUESHIELD with different control pins and with low byte shifted by one pin: PC2..PC9, as used by the rDuinoScope board
//
// IMPORTANT : make sure the jumpers are set correctly for the supply voltages of your TFT, do measure for safety!

#if defined(ARDUINO_ARCH_SAM)

#include "GxIO_DUE_P16_R_SHIELD.h"

GxIO_DUE_P16_R_SHIELD::GxIO_DUE_P16_R_SHIELD()
{
  _cs   = 31; // PA7
  _rs   = 22; // PB26
  _rst  = 28; // PD3
  _wr   = 23; // PA14
  _rd   = 24; // PA15
}

void GxIO_DUE_P16_R_SHIELD::reset()
{
  digitalWrite(_rst, LOW);
  delay(10);
  digitalWrite(_rst, HIGH);
  delay(10);
}

void GxIO_DUE_P16_R_SHIELD::init()
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

// D15 D14 D13 D12 D11 D10 D09 D08 D07 D06 D05 D04 D03 D02 D01 D00
// C19 C18 C17 C16 C15 C14 C13 C12 C09 C08 C07 C06 C05 C04 C03 C02

void GxIO_DUE_P16_R_SHIELD::setDataPins(uint8_t mode)
{
  pinMode(34, mode); // D0  : PC2
  pinMode(35, mode); // D1
  pinMode(36, mode); // D2
  pinMode(37, mode); // D3
  pinMode(38, mode); // D4
  pinMode(39, mode); // D5
  pinMode(40, mode); // D6 
  pinMode(41, mode); // D7  : PC9
  pinMode(51, mode); // D8  : PC12
  pinMode(50, mode); // D9
  pinMode(49, mode); // D10
  pinMode(48, mode); // D11
  pinMode(47, mode); // D12
  pinMode(46, mode); // D13
  pinMode(45, mode); // D14
  pinMode(44, mode); // D15 : PC19
}

uint8_t GxIO_DUE_P16_R_SHIELD::readDataTransaction()
{
  return readData16Transaction();
}

uint16_t GxIO_DUE_P16_R_SHIELD::readData16Transaction()
{
  REG_PIOA_CODR = 0x1 << 7; // PA7 CS_L
  uint16_t rv = readData16();
  REG_PIOA_SODR = 0x1 << 7; // PA7 CS_H
  return rv;
}

uint8_t GxIO_DUE_P16_R_SHIELD::readData()
{
  return readData16();
}

uint16_t GxIO_DUE_P16_R_SHIELD::readData16()
{
  setDataPins(INPUT);
  REG_PIOA_CODR = 0x1 << 15; // PA15 _rd = L
  REG_PIOA_CODR = 0x1 << 15;
  //REG_PIOA_CODR = 0x1 << 15; 
  //REG_PIOA_CODR = 0x1 << 15; 
  uint32_t rv = ((REG_PIOC_PDSR >> 2) & 0xFF) | ((REG_PIOC_PDSR >> (12 - 8)) & 0xFF00);
  REG_PIOA_SODR = 0x1 << 15; //PA15 _rd = H
  REG_PIOA_SODR = 0x1 << 15; //PA15 _rd = H
  setDataPins(OUTPUT);
  return rv;
}

uint32_t GxIO_DUE_P16_R_SHIELD::readRawData32(uint8_t part)
{
  setDataPins(INPUT);
  REG_PIOA_CODR = 0x1 << 15; // PA15 _rd = L
  REG_PIOA_CODR = 0x1 << 15; // PA15
  uint32_t rv = REG_PIOC_PDSR;
  REG_PIOA_SODR = 0x1 << 15; //PA15 _rd = H
  setDataPins(OUTPUT);
  return rv;
}

void GxIO_DUE_P16_R_SHIELD::writeCommandTransaction(uint8_t c)
{
  REG_PIOA_CODR = 0x1 << 7; // PA7 CS_L
  REG_PIOB_CODR = 0x1 << 26; // PB26 RS_L
  writeData16(c);
  REG_PIOB_SODR = 0x1 << 26; // PB26 RS_H
  REG_PIOA_SODR = 0x1 << 7; // PA7 CS_H
}

void GxIO_DUE_P16_R_SHIELD::writeCommand16Transaction(uint16_t c)
{
  REG_PIOB_CODR = 0x1 << 26; // PB26 RS_L
  REG_PIOA_CODR = 0x1 << 7; // PA7 CS_L
  writeData16(c);
  REG_PIOA_SODR = 0x1 << 7; // PA7 CS_H
  REG_PIOB_SODR = 0x1 << 26; // PB26 RS_H
}

void GxIO_DUE_P16_R_SHIELD::writeDataTransaction(uint8_t d)
{
  REG_PIOA_CODR = 0x1 << 7; // PA7 CS_L
  writeData16(d);
  REG_PIOA_SODR = 0x1 << 7; // PA7 CS_H
}

void GxIO_DUE_P16_R_SHIELD::writeData16Transaction(uint16_t d, uint32_t num)
{
  REG_PIOA_CODR = 0x1 << 7; // PA7 CS_L
  writeData16(d, num);
  REG_PIOA_SODR = 0x1 << 7; // PA7 CS_H
}

void GxIO_DUE_P16_R_SHIELD::writeCommand(uint8_t c)
{
  REG_PIOB_CODR = 0x1 << 26; // PB26 RS_L
  writeData16(c);
  REG_PIOB_SODR = 0x1 << 26; // PB26 RS_H
}

void GxIO_DUE_P16_R_SHIELD::writeCommand16(uint16_t c)
{
  REG_PIOB_CODR = 0x1 << 26; // PB26 RS_L
  writeData16(c);
  REG_PIOB_SODR = 0x1 << 26; // PB26 RS_H
}

void GxIO_DUE_P16_R_SHIELD::writeData(uint8_t d)
{
  writeData16(d);
}

void GxIO_DUE_P16_R_SHIELD::writeData(uint8_t* d, uint32_t num)
{
  while (num > 0)
  {
    writeData16(*d);
    d++;
    num--;
  }
}

void GxIO_DUE_P16_R_SHIELD::writeData16(uint16_t d, uint32_t num)
{
  REG_PIOC_CODR = (0xFF << 2) | (0xFF << 12); // PC2..PC9, PC12..PC19
  REG_PIOC_SODR = ((d & 0x00FF) << 2) | ((d & 0xFF00) << (12 - 8));
  while (num > 0)
  {
    REG_PIOA_CODR = 0x1 << 14; // PA14 = _wr -> L
    REG_PIOA_CODR = 0x1 << 14;
    REG_PIOA_CODR = 0x1 << 14;
    
    REG_PIOA_SODR = 0x1 << 14; // PA14 = _wr -> H
    num--;
  }
}

void GxIO_DUE_P16_R_SHIELD::writeAddrMSBfirst(uint16_t d)
{
  writeData16(d >> 8);
  writeData16(d & 0xFF);
}

void GxIO_DUE_P16_R_SHIELD::startTransaction()
{
  REG_PIOA_CODR = 0x1 << 7; // PA7 CS_L
}

void GxIO_DUE_P16_R_SHIELD::endTransaction()
{
  REG_PIOA_SODR = 0x1 << 7; // PA7 CS_H
}

void GxIO_DUE_P16_R_SHIELD::selectRegister(bool rs_low)
{
  if (rs_low) REG_PIOB_CODR = 0x1 << 26; // PB26 RS_L
  else REG_PIOB_SODR = 0x1 << 26; // PB26 RS_H
}

void GxIO_DUE_P16_R_SHIELD::setBackLight(bool lit)
{
  if (_bl >= 0) digitalWrite(_bl, (lit ? HIGH : LOW));
}

#endif

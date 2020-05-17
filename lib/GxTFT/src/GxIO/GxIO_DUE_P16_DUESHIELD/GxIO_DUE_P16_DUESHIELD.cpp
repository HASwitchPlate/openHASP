// created by Jean-Marc Zingg to be the GxIO_DUE_P16_DUESHIELD io class for the GxTFT library
// code extracts taken from https://github.com/Bodmer/TFT_HX8357_Due
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// for 16 bit parallel displays on shields or on adapter shields for DUE, e.g. CTE TFT LCD/SD Shield for Arduino Due
// e.g. https://www.aliexpress.com/item/New-TFT-SD-Shield-for-Arduino-DUE-TFT-LCD-Module-SD-Card-Adapter-2-8-3/32709157722.html
// for read functions to work, a connection from an Arduino pin to the LCD_RD signal needs to be made
// tested with 7" SSD1963 TFT, e.g. https://www.aliexpress.com/item/New-7-inch-TFT-LCD-module-800x480-SSD1963-Touch-PWM-For-Arduino-AVR-STM32-ARM/32667404985.html
//
// IMPORTANT : make sure the jumpers are set correctly for the supply voltages of your TFT, do measure for safety!

#if defined(ARDUINO_ARCH_SAM)

#include "GxIO_DUE_P16_DUESHIELD.h"

GxIO_DUE_P16_DUESHIELD::GxIO_DUE_P16_DUESHIELD()
{
  _cs   = 27; // PD2
  _rs   = 25; // PD0
  _rst  = 28; // PD3
  _wr   = 26; // PD1
  _rd   = 24; // PA15 : add wire from Arduino connector CN1 pin 5 to R3 LCD_RD on CTE shield
}

void GxIO_DUE_P16_DUESHIELD::reset()
{
  digitalWrite(_rst, LOW);
  delay(10);
  digitalWrite(_rst, HIGH);
  delay(10);
}

void GxIO_DUE_P16_DUESHIELD::init()
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

void GxIO_DUE_P16_DUESHIELD::setDataPins(uint8_t mode)
{
  pinMode(33, mode); // D0 : PC1
  pinMode(34, mode); // D1
  pinMode(35, mode); // D2
  pinMode(36, mode); // D3
  pinMode(37, mode); // D4
  pinMode(38, mode); // D5
  pinMode(39, mode); // D6 : PC7
  pinMode(40, mode); // D7 : PC8
  pinMode(44, mode); // D15 : PC19
  pinMode(45, mode); // D14
  pinMode(46, mode); // D13
  pinMode(47, mode); // D12
  pinMode(48, mode); // D11
  pinMode(49, mode); // D10
  pinMode(50, mode); // D9
  pinMode(51, mode); // D8 : PC12
}

uint8_t GxIO_DUE_P16_DUESHIELD::readDataTransaction()
{
  return readData16Transaction();
}

uint16_t GxIO_DUE_P16_DUESHIELD::readData16Transaction()
{
  REG_PIOD_CODR = 0x1 << 2; // PD2 CS_L
  uint16_t rv = readData16();
  REG_PIOD_SODR = 0x1 << 2; // PD2 CS_H
  return rv;
}

uint8_t GxIO_DUE_P16_DUESHIELD::readData()
{
  return readData16();
}

uint16_t GxIO_DUE_P16_DUESHIELD::readData16()
{
  setDataPins(INPUT);
  REG_PIOA_CODR = 0x1 << 15; // PA15
  REG_PIOA_CODR = 0x1 << 15; // PA15
  //REG_PIOA_CODR = 0x1 << 15; // PA15
  //REG_PIOA_CODR = 0x1 << 15; // PA15
  uint32_t rv = ((REG_PIOC_PDSR >> 1) & 0xFF) | ((REG_PIOC_PDSR >> (12 - 8)) & 0xFF00);
  REG_PIOA_SODR = 0x1 << 15; //PA15
  setDataPins(OUTPUT);
  return rv;
}

uint32_t GxIO_DUE_P16_DUESHIELD::readRawData32(uint8_t part)
{
  setDataPins(INPUT);
  REG_PIOA_CODR = 0x1 << 15; // PA15
  REG_PIOA_CODR = 0x1 << 15; // PA15
  uint32_t rv = REG_PIOC_PDSR;
  REG_PIOA_SODR = 0x1 << 15; //PA15
  setDataPins(OUTPUT);
  return rv;
}

void GxIO_DUE_P16_DUESHIELD::writeCommandTransaction(uint8_t c)
{
  REG_PIOD_CODR = 0x1 << 2; // PD2 CS_L
  REG_PIOD_CODR = 0x1 << 0; // PD0 RS_L
  writeData16(c);
  REG_PIOD_SODR = 0x1 << 0; // PD0 RS_H
  REG_PIOD_SODR = 0x1 << 2; // PD2 CS_H
}

void GxIO_DUE_P16_DUESHIELD::writeCommand16Transaction(uint16_t c)
{
  REG_PIOD_CODR = 0x1 << 2; // PD2 CS_L
  REG_PIOD_CODR = 0x1 << 0; // PD0 RS_L
  writeData16(c);
  REG_PIOD_SODR = 0x1 << 0; // PD0 RS_H
  REG_PIOD_SODR = 0x1 << 2; // PD2 CS_H
}

void GxIO_DUE_P16_DUESHIELD::writeDataTransaction(uint8_t d)
{
  REG_PIOD_CODR = 0x1 << 2; // PD2 CS_L
  writeData16(d);
  REG_PIOD_SODR = 0x1 << 2; // PD2 CS_H
}

void GxIO_DUE_P16_DUESHIELD::writeData16Transaction(uint16_t d, uint32_t num)
{
  REG_PIOD_CODR = 0x1 << 2; // PD2 CS_L
  writeData16(d, num);
  REG_PIOD_SODR = 0x1 << 2; // PD2 CS_H
}

void GxIO_DUE_P16_DUESHIELD::writeCommand(uint8_t c)
{
  REG_PIOD_CODR = 0x1 << 0; // PD0 RS_L
  writeData16(c);
  REG_PIOD_SODR = 0x1 << 0; // PD0 RS_H
}

void GxIO_DUE_P16_DUESHIELD::writeCommand16(uint16_t c)
{
  REG_PIOD_CODR = 0x1 << 0; // PD0 RS_L
  writeData16(c);
  REG_PIOD_SODR = 0x1 << 0; // PD0 RS_H
}

void GxIO_DUE_P16_DUESHIELD::writeData(uint8_t d)
{
  writeData16(d);
}

void GxIO_DUE_P16_DUESHIELD::writeData(uint8_t* d, uint32_t num)
{
  while (num > 0)
  {
    writeData16(*d);
    d++;
    num--;
  }
}

void GxIO_DUE_P16_DUESHIELD::writeData16(uint16_t d, uint32_t num)
{
  REG_PIOC_CODR = (0xFF << 1) | (0xFF << 12); // PC1..PC8, PC12..PC19
  REG_PIOC_SODR = ((d & 0x00FF) << 1) | ((d & 0xFF00) << (12 - 8));
  while (num > 0)
  {
    REG_PIOD_CODR = 0x1 << 1;
    REG_PIOD_CODR = 0x1 << 1;
    REG_PIOD_CODR = 0x1 << 1;
    REG_PIOD_SODR = 0x1 << 1;
    num--;
  }
}

void GxIO_DUE_P16_DUESHIELD::writeAddrMSBfirst(uint16_t d)
{
  writeData16(d >> 8);
  writeData16(d & 0xFF);
}

void GxIO_DUE_P16_DUESHIELD::startTransaction()
{
  REG_PIOD_CODR = 0x1 << 2; // PD2 CS_L
}

void GxIO_DUE_P16_DUESHIELD::endTransaction()
{
  REG_PIOD_SODR = 0x1 << 2; // PD2 CS_H
}

void GxIO_DUE_P16_DUESHIELD::selectRegister(bool rs_low)
{
  if (rs_low) REG_PIOD_CODR = 0x1 << 0; // PD0 RS_L
  else REG_PIOD_SODR = 0x1 << 0; // PD0 RS_H
}

void GxIO_DUE_P16_DUESHIELD::setBackLight(bool lit)
{
  if (_bl >= 0) digitalWrite(_bl, (lit ? HIGH : LOW));
}

#endif

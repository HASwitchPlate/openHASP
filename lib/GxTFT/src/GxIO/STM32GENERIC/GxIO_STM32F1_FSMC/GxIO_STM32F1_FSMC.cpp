// created by Jean-Marc Zingg to be the GxIO_STM32F1_FSMC io class for the GxTFT library
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// this is the io class for STM32F103VC boards with FMSC TFT connector, e.g. for 
// https://www.aliexpress.com/item/STM32-core-development-TFT-LCD-screen-evaluation-board-with-high-speed-FSMC-SDIO-interface/32667841009.html
//
// the above board has matched TFT connector for this display (aka Tiky)
// https://www.aliexpress.com/item/5-0-inch-HD-IPS-TFT-LCD-module-resistance-touch-with-PCB-adapter-board-854-480/32666829945.html
//
// for pin information see the backside of the TFT, for the data pin to port pin mapping see FSMC pin table STM32F103V doc.
//
// this io class can be used with or adapted to other STM32F103V processor boards with FSMC TFT connector.
//
// this version is for use with Arduino package STM32GENERIC, works with board "huaweiwx F103Z boards" variants "RedBull(F103ZE)" or "GENERIC(STM32F103ZE)".
// https://github.com/danieleff/STM32GENERIC
// settings used:
// USB: "Disabled, no USB", Serial communication: "SerialUART1", JTAG/SWD debugging: "JTAG [debug pins reserved: PB4,PB3,PA15,PA14,PA13]", Upload method: "STlink"
//
// also tried: DO NOT USE! DO NOT DOWNLOAD! DON'T ignore linker warning! -> flash erase (ST-Link utility) required afterwards!
// Arduino package STM32GENERIC, board "Blue F103VE/ZE/ZG boards" (ST-Link download failure). DO NOT USE! (version downloaded 7.7.2017)

#if defined(ARDUINO_ARCH_STM32) && (defined(STM32F103VE) || defined(STM32F103ZE) || defined(STM32F103ZG))

#include "GxIO_STM32F1_FSMC.h"

//                    |       |        | // Ruler on 8 & 16
#define PD_DATA_BITS 0b1100011100000011
#define PD_CTRL_BITS 0b0000100010110000  // PD11(FSMC_A16),PD7(FSMC_NE1),PD5(FSMC_NWE),PD4(FSMC_NOE)
#define PD_CRH_MASK  0xFF00FFFF
#define PD_CRH_FSMC  0xBB00BBBB          // AF PP 10MHz
#define PD_CRL_MASK          0xF0FF00FF
#define PD_CRL_FSMC          0xB0BB00BB  // AF PP 10MHz
//                    |       |        |
#define PE_DATA_BITS 0b1111111110000000
//                    |       |        |
#define PE_CRH_MASK  0xFFFFFFFF          // data bits used 15..8
#define PE_CRH_FSMC  0xBBBBBBBB          // AF PP 10MHz
#define PE_CRL_MASK          0xF0000000  // data bits used 7..0
#define PE_CRL_FSMC          0xB0000000  // AF PP 10MHz
//                    |       |        |

// 80 MHz processor clock
#define ADDSET 7 // 100ns (ADDSET+1)*12.5ns = CS to RW
#define DATAST 3 // 50ns  (DATAST+1)*12.5ns = RW length

#define CommandAccess FSMC_BANK1
#define DataAccess (FSMC_BANK1 + 0x20000)

GxIO_STM32F1_FSMC::GxIO_STM32F1_FSMC()
{
  _cs   = PD7;  // FSMC_NE1
  _rs   = PD11; // FSMC_A16
  _rst  = PD13;
  _wr   = PD5;  // FSMC_NWE
  _rd   = PD4;  // FSMC_NOE
  _bl   = PA1;
}

void GxIO_STM32F1_FSMC::reset()
{
  digitalWrite(_rst, LOW);
  delay(10);
  digitalWrite(_rst, HIGH);
  delay(10);
}

void GxIO_STM32F1_FSMC::init()
{
  RCC->AHBENR |= 0x00000100; // enable FSMC clock
  uint32_t t = RCC->AHBENR; // delay
  RCC->APB2ENR |= 0x00000060; // enable port D & E clocks
  t = RCC->APB2ENR; // delay
  (void)(t);
  GPIOD->CRH = (GPIOD->CRH & ~PD_CRH_MASK) | PD_CRH_FSMC;
  GPIOD->CRL = (GPIOD->CRL & ~PD_CRL_MASK) | PD_CRL_FSMC;
  GPIOE->CRH = (GPIOE->CRH & ~PE_CRH_MASK) | PE_CRH_FSMC;
  GPIOE->CRL = (GPIOE->CRL & ~PE_CRL_MASK) | PE_CRL_FSMC;
  FSMC_Bank1->BTCR[0] = 0x000010D9;
  FSMC_Bank1->BTCR[1] = (DATAST << 8) | ADDSET;
  digitalWrite(_rst, HIGH);
  digitalWrite(_bl, LOW);
  pinMode(_rst, OUTPUT);
  pinMode(_bl, OUTPUT);
  reset();
}

uint8_t GxIO_STM32F1_FSMC::readDataTransaction()
{
  return *(uint8_t*)DataAccess;
}

uint16_t GxIO_STM32F1_FSMC::readData16Transaction()
{
  return *(uint16_t*)DataAccess;
}

uint8_t GxIO_STM32F1_FSMC::readData()
{
  return *(uint8_t*)DataAccess;
}

uint16_t GxIO_STM32F1_FSMC::readData16()
{
  return *(uint16_t*)DataAccess;
}

uint32_t GxIO_STM32F1_FSMC::readRawData32(uint8_t part)
{
  return *(uint16_t*)DataAccess;
}

void GxIO_STM32F1_FSMC::writeCommandTransaction(uint8_t c)
{
  *(uint8_t*)CommandAccess = c;
}

void GxIO_STM32F1_FSMC::writeCommand16Transaction(uint16_t c)
{
  *(uint16_t*)CommandAccess = c;
}

void GxIO_STM32F1_FSMC::writeDataTransaction(uint8_t d)
{
  *(uint8_t*)DataAccess = d;
}

void GxIO_STM32F1_FSMC::writeData16Transaction(uint16_t d, uint32_t num)
{
  while (num > 0)
  {
    *(uint16_t*)DataAccess = d;
    num--;
  }
}

void GxIO_STM32F1_FSMC::writeCommand(uint8_t c)
{
  *(uint8_t*)CommandAccess = c;
}

void GxIO_STM32F1_FSMC::writeCommand16(uint16_t c)
{
  *(uint16_t*)CommandAccess = c;
}

void GxIO_STM32F1_FSMC::writeData(uint8_t d)
{
  *(uint8_t*)DataAccess = d;
}

void GxIO_STM32F1_FSMC::writeData(uint8_t* d, uint32_t num)
{
  while (num > 0)
  {
    *(uint8_t*)DataAccess = *d;
    d++;
    num--;
  }
}

void GxIO_STM32F1_FSMC::writeData16(uint16_t d, uint32_t num)
{
  while (num > 0)
  {
    *(uint16_t*)DataAccess = d;
    num--;
  }
}

void GxIO_STM32F1_FSMC::writeAddrMSBfirst(uint16_t d)
{
  writeData16(d >> 8);
  writeData16(d & 0xFF);
}

void GxIO_STM32F1_FSMC::startTransaction()
{
}

void GxIO_STM32F1_FSMC::endTransaction()
{
}

void GxIO_STM32F1_FSMC::selectRegister(bool rs_low)
{
}

void GxIO_STM32F1_FSMC::setBackLight(bool lit)
{
  digitalWrite(_bl, (lit ? HIGH : LOW));
}

#endif

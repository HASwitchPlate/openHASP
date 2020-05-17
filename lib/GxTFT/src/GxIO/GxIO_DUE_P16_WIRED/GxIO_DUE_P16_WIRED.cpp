// created by Jean-Marc Zingg to be the GxIO_DUE_P16_WIRED io class for the GxTFT library
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// for 7inch display https://www.aliexpress.com/item/New-7-inch-TFT-LCD-module-800x480-SSD1963-Touch-PWM-For-Arduino-AVR-STM32-ARM/32667404985.html
//
// and IPS 3.97inch OTM8009A https://www.aliexpress.com/item/IPS-3-97-inch-HD-TFT-LCD-Touch-Screen-Module-OTM8009A-Drive-IC-800-480/32676929794.html

#if defined(ARDUINO_ARCH_SAM)

#include "GxIO_DUE_P16_WIRED.h"

// connector pins
// 01  02  03  04  05  06  07  08  09  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36   37  38  39  40
// GND D0  3.3 D1  NC  D2  RS  D3  WR  D4  RD  D5  D8  D6  D9  D7  D10 TCK D11 TCS D12 TDI D13 NC  D14 TDO D15 TIR CS  SDO FCS SDC RST SDI 5V  SDCS LED NC  NC  NC
// to Due pins
// GND 52  3.3 50  53  48  51  46  49  44  47  42  45  40  43  38  41  36  39  34  37  32  35  30  33  28  31  26  29  24  27  22  25  NC  5V  NC   23
//     B21     C13 B14 C15 C12 C17 C14 C19 C16 A19 C18 C8  A20 C6  C9  C4  C7  C2  C5  D10 C3  D9  C1  D3  A7  D1  D6  A15 D1  B26 D0               A14

//        : |..,..,..,..|..,..,..,..|..,..,..,..|..,..,..,..|..,..,..,..|..,..,..,..|..,..,..,..|..,..,..,..|
//        : |31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
// A data : |..,..,..,..|..,..,..,..|..,..,..,20|19,..,..,..|..,..,..,..|..,..,..,..| 7,..,..,..|..,..,..,..| #3
// B data : |..,..,..,..|..,..,..,..|..,..,21,..|..,..,..,..|..,..,..,..|..,..,..,..|..,..,..,..|..,..,..,..| #1
// C data : |..,..,..,..|..,..,..,..|..,..,..,..|19,18,17,..|15,..,13,..|..,.., 9, 8| 7, 6, 5,..| 3,.., 1,..| #12
// D data : |..,..,..,..|..,..,..,..|..,..,..,..|..,..,..,..|..,..,..,..|..,..,..,..|..,..,..,..|..,..,..,..| #0

#define PA_DATA_BITS 0x00180080
#define PB_DATA_BITS 0x00200000
#define PC_DATA_BITS 0x000EA3EA
#define PD_DATA_BITS 0x00000000

GxIO_DUE_P16_WIRED::GxIO_DUE_P16_WIRED(bool bl_active_high)
{
  _cs   = 29;
  _rs   = 51;
  _rst  = 25;
  _wr   = 49;
  _rd   = 47;
  _bl   = 23;
  _bl_active_high = bl_active_high;
}

void GxIO_DUE_P16_WIRED::reset()
{
  // _rst pin not available
}

void GxIO_DUE_P16_WIRED::init()
{
  digitalWrite(_rst, HIGH);
  digitalWrite(_rs, HIGH);
  digitalWrite(_cs, HIGH);
  digitalWrite(_wr, HIGH);
  digitalWrite(_rd, HIGH);
  digitalWrite(_bl, LOW);
  pinMode(_bl, OUTPUT);
  pinMode(_rst, OUTPUT);
  pinMode(_rs, OUTPUT);
  pinMode(_cs, OUTPUT);
  pinMode(_wr, OUTPUT);
  pinMode(_rd, OUTPUT);
  reset();
  // Set 16 bit port pins to output on Due
  setDataPins(OUTPUT);
}

void GxIO_DUE_P16_WIRED::setDataPins(uint8_t mode)
{
  pinMode(52, mode);
  pinMode(50, mode);
  pinMode(48, mode);
  pinMode(46, mode);
  pinMode(44, mode);
  pinMode(42, mode);
  pinMode(45, mode);
  pinMode(40, mode);
  pinMode(43, mode);
  pinMode(38, mode);
  pinMode(41, mode);
  pinMode(39, mode);
  pinMode(37, mode);
  pinMode(35, mode);
  pinMode(33, mode);
  pinMode(31, mode);
}

uint8_t GxIO_DUE_P16_WIRED::readDataTransaction()
{
  return readData16Transaction();
}

uint16_t GxIO_DUE_P16_WIRED::readData16Transaction()
{
  digitalWrite(_cs, LOW);
  uint16_t rv = readData16();
  digitalWrite(_cs, HIGH);
  return rv;
}

uint8_t GxIO_DUE_P16_WIRED::readData()
{
  return readData16();
}

uint16_t GxIO_DUE_P16_WIRED::readData16()
{
  setDataPins(INPUT);
  delayMicroseconds(1);
  digitalWrite(_rd, LOW);
  digitalWrite(_rd, HIGH);
  digitalWrite(_rd, HIGH);
  digitalWrite(_rd, LOW);
  digitalWrite(_rd, LOW);
  delayMicroseconds(10);
  uint32_t rv = 0;
  // The compiler efficiently codes this
  // so it is quite quick.                        Port.bit
  rv |= (REG_PIOA_PDSR & (0x1 << 7)) << (15 - 7); // A7
  rv |= (REG_PIOC_PDSR & (0x1 << 1)) << (14 - 1); // C1
  rv |= (REG_PIOC_PDSR & (0x1 << 3)) << (13 - 3); // C3
  rv |= (REG_PIOC_PDSR & (0x1 << 5)) << (12 - 5); // C5
  rv |= (REG_PIOC_PDSR & (0x1 << 7)) << (11 - 7); // C7
  rv |= (REG_PIOC_PDSR & (0x1 << 9)) << (10 - 9); // C9
  rv |= (REG_PIOA_PDSR & (0x1 << 20)) >> -(9 - 20); // A20
  rv |= (REG_PIOC_PDSR & (0x1 << 18)) >> -(8 - 18); // C18
  // so it is quite quick.                        Port.bit
  rv |= (REG_PIOC_PDSR & (0x1 << 6)) << (7 - 6); // C6
  rv |= (REG_PIOC_PDSR & (0x1 << 8)) >> -(6 - 8); // C8
  rv |= (REG_PIOA_PDSR & (0x1 << 19)) >> -(5 - 19); // A19
  rv |= (REG_PIOC_PDSR & (0x1 << 19)) >> -(4 - 19); // C19
  rv |= (REG_PIOC_PDSR & (0x1 << 17)) >> -(3 - 17); // C17
  rv |= (REG_PIOC_PDSR & (0x1 << 15)) >> -(2 - 15); // C15
  rv |= (REG_PIOC_PDSR & (0x1 << 13)) >> -(1 - 13); // C13
  rv |= (REG_PIOB_PDSR & (0x1 << 21)) >> -(0 - 21); // B21
  digitalWrite(_rd, HIGH);
  setDataPins(OUTPUT);
  return rv;
}

uint32_t GxIO_DUE_P16_WIRED::readRawData32(uint8_t part)
{
  setDataPins(INPUT);
  delayMicroseconds(1);
  digitalWrite(_rd, LOW);
  digitalWrite(_rd, HIGH);
  digitalWrite(_rd, HIGH);
  digitalWrite(_rd, LOW);
  digitalWrite(_rd, LOW);
  delayMicroseconds(1);
  uint32_t rv = 0;
  if (part == 0) rv = REG_PIOA_PDSR;
  if (part == 1) rv = REG_PIOC_PDSR;
  if (part == 2) rv = REG_PIOD_PDSR;
  digitalWrite(_rd, HIGH);
  setDataPins(OUTPUT);
  return rv;
}

void GxIO_DUE_P16_WIRED::writeCommandTransaction(uint8_t c)
{
  digitalWrite(_cs, LOW);
  digitalWrite(_rs, LOW);
  writeData16(c);
  digitalWrite(_rs, HIGH);
  digitalWrite(_cs, HIGH);
}

void GxIO_DUE_P16_WIRED::writeCommand16Transaction(uint16_t c)
{
  digitalWrite(_cs, LOW);
  digitalWrite(_rs, LOW);
  writeData16(c);
  digitalWrite(_rs, HIGH);
  digitalWrite(_cs, HIGH);
}

void GxIO_DUE_P16_WIRED::writeDataTransaction(uint8_t d)
{
  digitalWrite(_cs, LOW);
  writeData16(d);
  digitalWrite(_cs, HIGH);
}

void GxIO_DUE_P16_WIRED::writeData16Transaction(uint16_t d, uint32_t num)
{
  digitalWrite(_cs, LOW);
  writeData16(d, num);
  digitalWrite(_cs, HIGH);
}

void GxIO_DUE_P16_WIRED::writeCommand(uint8_t c)
{
  digitalWrite(_rs, LOW);
  writeData16(c);
  digitalWrite(_rs, HIGH);
}

void GxIO_DUE_P16_WIRED::writeCommand16(uint16_t c)
{
  digitalWrite(_rs, LOW);
  writeData16(c);
  digitalWrite(_rs, HIGH);
}

void GxIO_DUE_P16_WIRED::writeData(uint8_t d)
{
  writeData16(d);
}

void GxIO_DUE_P16_WIRED::writeData(uint8_t* d, uint32_t num)
{
  while (num > 0)
  {
    writeData16(*d);
    d++;
    num--;
  }
}

void GxIO_DUE_P16_WIRED::writeData16(uint16_t d, uint32_t num)
{
  REG_PIOA_CODR = PA_DATA_BITS; // Clear bits
  REG_PIOB_CODR = PB_DATA_BITS; // Clear bits
  REG_PIOC_CODR = PC_DATA_BITS; // Clear bits
  REG_PIOD_CODR = PD_DATA_BITS; // Clear bits

// connector pins
// 01  02  03  04  05  06  07  08  09  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36   37  38  39  40
// GND D0  3.3 D1  NC  D2  RS  D3  WR  D4  RD  D5  D8  D6  D9  D7  D10 TCK D11 TCS D12 TDI D13 NC  D14 TDO D15 TIR CS  SDO FCS SDC RST SDI 5V  SDCS LED NC  NC  NC
// to Due pins
// GND 52  3.3 50  53  48  51  46  49  44  47  42  45  40  43  38  41  36  39  34  37  32  35  30  33  28  31  26  29  24  27  22  25  NC  5V  NC   23
//     B21     C13 B14 C15 C12 C17 C14 C19 C16 A19 C18 C8  A20 C6  C9  C4  C7  C2  C5  D10 C3  D9  C1  D3  A7  D1  D6  A15 D1  B26 D0               A14
  // The compiler efficiently codes this
  // so it is quite quick.                    Port.bit
  if (d & 0x8000) REG_PIOA_SODR = 0x1 << 7; // A7
  if (d & 0x4000) REG_PIOC_SODR = 0x1 << 1; // C1
  if (d & 0x2000) REG_PIOC_SODR = 0x1 << 3; // C3
  if (d & 0x1000) REG_PIOC_SODR = 0x1 << 5; // C5
  if (d & 0x0800) REG_PIOC_SODR = 0x1 << 7; // C7
  if (d & 0x0400) REG_PIOC_SODR = 0x1 << 9; // C9
  if (d & 0x0200) REG_PIOA_SODR = 0x1 << 20; // A20
  if (d & 0x0100) REG_PIOC_SODR = 0x1 << 18; // C18

  // so it is quite quick.                    Port.bit
  if (d & 0x0080) REG_PIOC_SODR = 0x1 << 6; // C6
  if (d & 0x0040) REG_PIOC_SODR = 0x1 << 8; // C8
  if (d & 0x0020) REG_PIOA_SODR = 0x1 << 19; // A19
  if (d & 0x0010) REG_PIOC_SODR = 0x1 << 19; // C19
  if (d & 0x0008) REG_PIOC_SODR = 0x1 << 17; // C17
  if (d & 0x0004) REG_PIOC_SODR = 0x1 << 15; // C15
  if (d & 0x0002) REG_PIOC_SODR = 0x1 << 13; // C13
  if (d & 0x0001) REG_PIOB_SODR = 0x1 << 21; // B21
  while (num > 0)
  {
    digitalWrite(_wr, LOW);
    digitalWrite(_wr, HIGH);
    num--;
  }
}

void GxIO_DUE_P16_WIRED::writeAddrMSBfirst(uint16_t d)
{
  writeData16(d >> 8);
  writeData16(d & 0xFF);
}

void GxIO_DUE_P16_WIRED::startTransaction()
{
  digitalWrite(_cs, LOW);
}

void GxIO_DUE_P16_WIRED::endTransaction()
{
  digitalWrite(_cs, HIGH);
}

void GxIO_DUE_P16_WIRED::selectRegister(bool rs_low)
{
  digitalWrite(_rs, (rs_low ? LOW : HIGH));
}

void GxIO_DUE_P16_WIRED::setBackLight(bool lit)
{
  digitalWrite(_bl, (lit == _bl_active_high));
}

#endif


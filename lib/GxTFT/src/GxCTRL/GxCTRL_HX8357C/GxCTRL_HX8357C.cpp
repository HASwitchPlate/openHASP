// created by Jean-Marc Zingg to be the GxCTRL_HX8357C class for the GxTFT library
// code extracts taken from https://github.com/Bodmer/TFT_HX8357
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// note: untested, and read functions are untested; I have no HX8357 display to test with

#include "GxCTRL_HX8357C.h"

#define CASET 0x2A
#define PASET 0x2B
#define RAMWR 0x2C
#define MADCTL 0x36
#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_SS  0x02
#define MADCTL_GS  0x01

uint32_t GxCTRL_HX8357C::readID()
{
  IO.writeCommandTransaction(0);
  return IO.readData16Transaction();
}

uint32_t GxCTRL_HX8357C::readRegister(uint8_t nr, uint8_t index, uint8_t bytes)
{
  uint32_t rv = 0;
  bytes = min(bytes, 4);
  for (uint8_t i = 0; i < bytes; i++)
  {
    IO.writeCommandTransaction(nr + index + i);
    rv |= IO.readDataTransaction();
  }
  return rv;
}

void GxCTRL_HX8357C::init()
{
  // HX8357-C display initialisation

  IO.writeCommandTransaction(0xB9); // Enable extension command
  IO.writeDataTransaction(0xFF);
  IO.writeDataTransaction(0x83);
  IO.writeDataTransaction(0x57);
  delay(50);

  IO.writeCommandTransaction(0xB6); //Set VCOM voltage
  IO.writeDataTransaction(0x2C);    //0x52 for HSD 3.0"

  IO.writeCommandTransaction(0x11); // Sleep off
  delay(200);

  IO.writeCommandTransaction(0x35); // Tearing effect on
  IO.writeDataTransaction(0x00);    // Added parameter

  IO.writeCommandTransaction(0x3A); // Interface pixel format
  IO.writeDataTransaction(0x55);    // 16 bits per pixel

  //IO.writeCommandTransaction(0xCC); // Set panel characteristic
  //IO.writeDataTransaction(0x09);    // S960>S1, G1>G480, R-G-B, normally black

  //IO.writeCommandTransaction(0xB3); // RGB interface
  //IO.writeDataTransaction(0x43);
  //IO.writeDataTransaction(0x00);
  //IO.writeDataTransaction(0x06);
  //IO.writeDataTransaction(0x06);

  IO.writeCommandTransaction(0xB1); // Power control
  IO.writeDataTransaction(0x00);
  IO.writeDataTransaction(0x15);
  IO.writeDataTransaction(0x0D);
  IO.writeDataTransaction(0x0D);
  IO.writeDataTransaction(0x83);
  IO.writeDataTransaction(0x48);


  IO.writeCommandTransaction(0xC0); // Does this do anything?
  IO.writeDataTransaction(0x24);
  IO.writeDataTransaction(0x24);
  IO.writeDataTransaction(0x01);
  IO.writeDataTransaction(0x3C);
  IO.writeDataTransaction(0xC8);
  IO.writeDataTransaction(0x08);

  IO.writeCommandTransaction(0xB4); // Display cycle
  IO.writeDataTransaction(0x02);
  IO.writeDataTransaction(0x40);
  IO.writeDataTransaction(0x00);
  IO.writeDataTransaction(0x2A);
  IO.writeDataTransaction(0x2A);
  IO.writeDataTransaction(0x0D);
  IO.writeDataTransaction(0x4F);

  IO.writeCommandTransaction(0xE0); // Gamma curve
  IO.writeDataTransaction(0x00);
  IO.writeDataTransaction(0x15);
  IO.writeDataTransaction(0x1D);
  IO.writeDataTransaction(0x2A);
  IO.writeDataTransaction(0x31);
  IO.writeDataTransaction(0x42);
  IO.writeDataTransaction(0x4C);
  IO.writeDataTransaction(0x53);
  IO.writeDataTransaction(0x45);
  IO.writeDataTransaction(0x40);
  IO.writeDataTransaction(0x3B);
  IO.writeDataTransaction(0x32);
  IO.writeDataTransaction(0x2E);
  IO.writeDataTransaction(0x28);

  IO.writeDataTransaction(0x24);
  IO.writeDataTransaction(0x03);
  IO.writeDataTransaction(0x00);
  IO.writeDataTransaction(0x15);
  IO.writeDataTransaction(0x1D);
  IO.writeDataTransaction(0x2A);
  IO.writeDataTransaction(0x31);
  IO.writeDataTransaction(0x42);
  IO.writeDataTransaction(0x4C);
  IO.writeDataTransaction(0x53);
  IO.writeDataTransaction(0x45);
  IO.writeDataTransaction(0x40);
  IO.writeDataTransaction(0x3B);
  IO.writeDataTransaction(0x32);

  IO.writeDataTransaction(0x2E);
  IO.writeDataTransaction(0x28);
  IO.writeDataTransaction(0x24);
  IO.writeDataTransaction(0x03);
  IO.writeDataTransaction(0x00);
  IO.writeDataTransaction(0x01);

  IO.writeCommandTransaction(0x36); // MADCTL Memory access control
  IO.writeDataTransaction(0x48);
  delay(20);

  IO.writeCommandTransaction(0x21); //Display inversion on
  delay(20);

  IO.writeCommandTransaction(0x29); // Display on

  delay(120);
}

void GxCTRL_HX8357C::setWindowAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
  IO.writeCommand(CASET);  // Column addr set
  IO.writeData(x0 >> 8);
  IO.writeData(x0 & 0xFF); // XSTART
  IO.writeData(x1 >> 8);
  IO.writeData(x1 & 0xFF); // XEND
  IO.writeCommand(PASET);  // Row addr set
  IO.writeData(y0 >> 8);
  IO.writeData(y0);        // YSTART
  IO.writeData(y1 >> 8);
  IO.writeData(y1);        // YEND
  IO.writeCommand(RAMWR);  // write to RAM
}

void GxCTRL_HX8357C::setRotation(uint8_t r)
{
  IO.startTransaction();
  IO.writeCommand(MADCTL);
  switch (r & 3)
  {
    case 0: // Portrait
      IO.writeData(MADCTL_BGR | MADCTL_MX);
      break;
    case 1: // Landscape (Portrait + 90)
      IO.writeData(MADCTL_BGR | MADCTL_MV);
      break;
    case 2: // Inverter portrait
      IO.writeData( MADCTL_BGR | MADCTL_MY);
      break;
    case 3: // Inverted landscape
      IO.writeData(MADCTL_BGR | MADCTL_MV | MADCTL_MX | MADCTL_MY);
      break;
  }
  IO.endTransaction();
}


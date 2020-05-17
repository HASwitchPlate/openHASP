// created by Jean-Marc Zingg to be the GxCTRL_RA8875S class for the GxTFT library
// code extracts taken from https://github.com/adafruit/Adafruit_RA8875
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// This controller class is to be used with GxIO_SPI
// It uses SPI calls that include the RS selection prefix
//
// note: readRect does not work correctly with my only RA8875 display (pixel sequence & garbage)
//       workaround added, but needs further investigation

#include "GxCTRL_RA8875S.h"

// Command/Data pins for SPI
#define RA8875_DATAWRITE        0x00
#define RA8875_DATAREAD         0x40
#define RA8875_CMDWRITE         0x80
#define RA8875_CMDREAD          0xC0

// Registers & bits
#define RA8875_PWRR             0x01
#define RA8875_PWRR_DISPON      0x80
#define RA8875_PWRR_DISPOFF     0x00
#define RA8875_PWRR_SLEEP       0x02
#define RA8875_PWRR_NORMAL      0x00
#define RA8875_PWRR_SOFTRESET   0x01

#define RA8875_MRWC             0x02

#define RA8875_GPIOX            0xC7

#define RA8875_PLLC1            0x88
#define RA8875_PLLC1_PLLDIV2    0x80
#define RA8875_PLLC1_PLLDIV1    0x00

#define RA8875_PLLC2            0x89
#define RA8875_PLLC2_DIV1       0x00
#define RA8875_PLLC2_DIV2       0x01
#define RA8875_PLLC2_DIV4       0x02
#define RA8875_PLLC2_DIV8       0x03
#define RA8875_PLLC2_DIV16      0x04
#define RA8875_PLLC2_DIV32      0x05
#define RA8875_PLLC2_DIV64      0x06
#define RA8875_PLLC2_DIV128     0x07

#define RA8875_SYSR             0x10
#define RA8875_SYSR_8BPP        0x00
#define RA8875_SYSR_16BPP       0x0C
#define RA8875_SYSR_MCU8        0x00
#define RA8875_SYSR_MCU16       0x03

#define RA8875_PCSR             0x04
#define RA8875_PCSR_PDATR       0x00
#define RA8875_PCSR_PDATL       0x80
#define RA8875_PCSR_CLK         0x00
#define RA8875_PCSR_2CLK        0x01
#define RA8875_PCSR_4CLK        0x02
#define RA8875_PCSR_8CLK        0x03

#define RA8875_HDWR             0x14

#define RA8875_HNDFTR           0x15
#define RA8875_HNDFTR_DE_HIGH   0x00
#define RA8875_HNDFTR_DE_LOW    0x80

#define RA8875_HNDR             0x16
#define RA8875_HSTR             0x17
#define RA8875_HPWR             0x18
#define RA8875_HPWR_LOW         0x00
#define RA8875_HPWR_HIGH        0x80

#define RA8875_VDHR0            0x19
#define RA8875_VDHR1            0x1A
#define RA8875_VNDR0            0x1B
#define RA8875_VNDR1            0x1C
#define RA8875_VSTR0            0x1D
#define RA8875_VSTR1            0x1E
#define RA8875_VPWR             0x1F
#define RA8875_VPWR_LOW         0x00
#define RA8875_VPWR_HIGH        0x80

#define RA8875_DPCR             0x20 // Display Configuration Register
#define RA8875_FNCR1            0x22 //Font Control Register 1

#define RA8875_HSAW0            0x30
#define RA8875_HSAW1            0x31
#define RA8875_VSAW0            0x32
#define RA8875_VSAW1            0x33

#define RA8875_HEAW0            0x34
#define RA8875_HEAW1            0x35
#define RA8875_VEAW0            0x36
#define RA8875_VEAW1            0x37

#define RA8875_MCLR             0x8E
#define RA8875_MCLR_START       0x80
#define RA8875_MCLR_STOP        0x00
#define RA8875_MCLR_READSTATUS  0x80
#define RA8875_MCLR_FULL        0x00
#define RA8875_MCLR_ACTIVE      0x40

#define RA8875_DCR                    0x90
#define RA8875_DCR_LINESQUTRI_START   0x80
#define RA8875_DCR_LINESQUTRI_STOP    0x00
#define RA8875_DCR_LINESQUTRI_STATUS  0x80
#define RA8875_DCR_CIRCLE_START       0x40
#define RA8875_DCR_CIRCLE_STATUS      0x40
#define RA8875_DCR_CIRCLE_STOP        0x00
#define RA8875_DCR_FILL               0x20
#define RA8875_DCR_NOFILL             0x00
#define RA8875_DCR_DRAWLINE           0x00
#define RA8875_DCR_DRAWTRIANGLE       0x01
#define RA8875_DCR_DRAWSQUARE         0x10


#define RA8875_ELLIPSE                0xA0
#define RA8875_ELLIPSE_STATUS         0x80

#define RA8875_MWCR0            0x40
#define RA8875_MWCR0_GFXMODE    0x00
#define RA8875_MWCR0_TXTMODE    0x80

#define RA8875_CURH0            0x46
#define RA8875_CURH1            0x47
#define RA8875_CURV0            0x48
#define RA8875_CURV1            0x49

#define RA8875_MRCD              0x45
#define RA8875_RCURH0            0x4A
#define RA8875_RCURH1            0x4B
#define RA8875_RCURV0            0x4C
#define RA8875_RCURV1            0x4D

#define RA8875_P1CR             0x8A
#define RA8875_P1CR_ENABLE      0x80
#define RA8875_P1CR_DISABLE     0x00
#define RA8875_P1CR_CLKOUT      0x10
#define RA8875_P1CR_PWMOUT      0x00

#define RA8875_P1DCR            0x8B

#define RA8875_P2CR             0x8C
#define RA8875_P2CR_ENABLE      0x80
#define RA8875_P2CR_DISABLE     0x00
#define RA8875_P2CR_CLKOUT      0x10
#define RA8875_P2CR_PWMOUT      0x00

#define RA8875_P2DCR            0x8D

#define RA8875_PWM_CLK_DIV1     0x00
#define RA8875_PWM_CLK_DIV2     0x01
#define RA8875_PWM_CLK_DIV4     0x02
#define RA8875_PWM_CLK_DIV8     0x03
#define RA8875_PWM_CLK_DIV16    0x04
#define RA8875_PWM_CLK_DIV32    0x05
#define RA8875_PWM_CLK_DIV64    0x06
#define RA8875_PWM_CLK_DIV128   0x07
#define RA8875_PWM_CLK_DIV256   0x08
#define RA8875_PWM_CLK_DIV512   0x09
#define RA8875_PWM_CLK_DIV1024  0x0A
#define RA8875_PWM_CLK_DIV2048  0x0B
#define RA8875_PWM_CLK_DIV4096  0x0C
#define RA8875_PWM_CLK_DIV8192  0x0D
#define RA8875_PWM_CLK_DIV16384 0x0E
#define RA8875_PWM_CLK_DIV32768 0x0F

#define RA8875_TPCR0                  0x70
#define RA8875_TPCR0_ENABLE           0x80
#define RA8875_TPCR0_DISABLE          0x00
#define RA8875_TPCR0_WAIT_512CLK      0x00
#define RA8875_TPCR0_WAIT_1024CLK     0x10
#define RA8875_TPCR0_WAIT_2048CLK     0x20
#define RA8875_TPCR0_WAIT_4096CLK     0x30
#define RA8875_TPCR0_WAIT_8192CLK     0x40
#define RA8875_TPCR0_WAIT_16384CLK    0x50
#define RA8875_TPCR0_WAIT_32768CLK    0x60
#define RA8875_TPCR0_WAIT_65536CLK    0x70
#define RA8875_TPCR0_WAKEENABLE       0x08
#define RA8875_TPCR0_WAKEDISABLE      0x00
#define RA8875_TPCR0_ADCCLK_DIV1      0x00
#define RA8875_TPCR0_ADCCLK_DIV2      0x01
#define RA8875_TPCR0_ADCCLK_DIV4      0x02
#define RA8875_TPCR0_ADCCLK_DIV8      0x03
#define RA8875_TPCR0_ADCCLK_DIV16     0x04
#define RA8875_TPCR0_ADCCLK_DIV32     0x05
#define RA8875_TPCR0_ADCCLK_DIV64     0x06
#define RA8875_TPCR0_ADCCLK_DIV128    0x07

#define RA8875_TPCR1            0x71
#define RA8875_TPCR1_AUTO       0x00
#define RA8875_TPCR1_MANUAL     0x40
#define RA8875_TPCR1_VREFINT    0x00
#define RA8875_TPCR1_VREFEXT    0x20
#define RA8875_TPCR1_DEBOUNCE   0x04
#define RA8875_TPCR1_NODEBOUNCE 0x00
#define RA8875_TPCR1_IDLE       0x00
#define RA8875_TPCR1_WAIT       0x01
#define RA8875_TPCR1_LATCHX     0x02
#define RA8875_TPCR1_LATCHY     0x03

#define RA8875_TPXH             0x72
#define RA8875_TPYH             0x73
#define RA8875_TPXYL            0x74

#define RA8875_INTC1            0xF0
#define RA8875_INTC1_KEY        0x10
#define RA8875_INTC1_DMA        0x08
#define RA8875_INTC1_TP         0x04
#define RA8875_INTC1_BTE        0x02

#define RA8875_INTC2            0xF1
#define RA8875_INTC2_KEY        0x10
#define RA8875_INTC2_DMA        0x08
#define RA8875_INTC2_TP         0x04
#define RA8875_INTC2_BTE        0x02

uint32_t GxCTRL_RA8875S::readID()
{
  IO.setFrequency(4000000); // slow down for read
  writeCommand(0x0);
  IO.startTransaction();
  IO.writeData(RA8875_DATAREAD);
  uint8_t dummy = IO.readData();
  uint32_t rv = IO.readData();
  IO.endTransaction();
  IO.setFrequency(16000000);
  return rv;
}

uint32_t GxCTRL_RA8875S::readRegister(uint8_t nr, uint8_t index, uint8_t bytes)
{
  uint32_t rv = 0;
  bytes = min(bytes, 4);
  IO.setFrequency(4000000); // slow down for read
  //for (uint8_t i = 0; i < bytes; i++)
  while (bytes > 0)
  {
    rv <<= 8;
    bytes--;
    writeCommand(nr + index + bytes);
    IO.startTransaction();
    IO.writeData(RA8875_DATAREAD);
    IO.readData16(); // dummy
    rv |= IO.readData();
    IO.endTransaction();
  }
  IO.setFrequency(16000000);
  return rv;
}

uint16_t GxCTRL_RA8875S::readPixel(uint16_t x, uint16_t y)
{
  uint16_t rv;
  readRect(x, y, 1, 1, &rv);
  return rv;
}

#if defined(RA8875_RAMRD_AUTO_INCREMENT_OK) // not ok on my display

void GxCTRL_RA8875S::readRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t* data)
{
  uint16_t xe = x + w - 1;
  uint16_t ye = y + h - 1;
  uint32_t num = uint32_t(w) * uint32_t(h);
  rotateWindow(x, y, xe, ye);
  _is_clipping = true; // no need to check for full screen
  writeReg16(RA8875_HSAW0, x); // horizontal start point
  writeReg16(RA8875_HEAW0, xe); // horizontal end point
  writeReg16(RA8875_VSAW0, y); // vertical start point
  writeReg16(RA8875_VEAW0, ye); // vertical end point
  writeReg16(RA8875_RCURH0, x); // horizontal read cursor
  writeReg16(RA8875_RCURV0, y); // vertical read cursor
  IO.setFrequency(4000000); // slow down for read
  IO.startTransaction();
  IO.writeData(RA8875_CMDWRITE);
  IO.writeData(RA8875_MRWC);
  IO.writeData(RA8875_DATAREAD);
  IO.readData16(); // dummy
  IO.readData16(); // dummy
  for (; num > 0; num--)
  {
    *data++ = IO.readData16();
  }
  IO.endTransaction();
  IO.setFrequency(16000000);
}

#else

void GxCTRL_RA8875S::readRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t* data)
{
  _is_clipping = true; // no need to check for full screen
  uint16_t xe = x + w - 1;
  uint16_t ye = y + h - 1;
  IO.setFrequency(4000000); // slow down for read
  for (uint16_t yy = y; yy <= ye; yy++)
  {
    for (uint16_t xx = x; xx <= xe; xx++)
    {
      uint16_t xxx = xx;
      uint16_t yyy = yy;
      rotatePoint(xxx, yyy);
      writeReg16(RA8875_HSAW0, xxx); // horizontal start point
      writeReg16(RA8875_HEAW0, xxx); // horizontal end point
      writeReg16(RA8875_VSAW0, yyy); // vertical start point
      writeReg16(RA8875_VEAW0, yyy); // vertical end point
      writeReg16(RA8875_RCURH0, xxx); // horizontal read cursor
      writeReg16(RA8875_RCURV0, yyy); // vertical read cursor
      IO.startTransaction();
      IO.writeData(RA8875_CMDWRITE);
      IO.writeData(RA8875_MRWC);
      IO.writeData(RA8875_DATAREAD);
      IO.readData16(); // dummy
      IO.readData16(); // dummy, why two?
      *data++ = IO.readData16();
      IO.endTransaction();
    }
  }
  IO.setFrequency(16000000);
}

#endif

void GxCTRL_RA8875S::init()
{
  _rotation = 1; // landscape is default
  IO.setFrequency(4000000);
  writeReg(RA8875_PLLC1, RA8875_PLLC1_PLLDIV1 + 10);
  delay(1);
  writeReg(RA8875_PLLC2, RA8875_PLLC2_DIV4);
  delay(1);
  writeReg(RA8875_SYSR, RA8875_SYSR_16BPP | RA8875_SYSR_MCU8);

  /* Timing values */
  uint8_t pixclk;
  uint8_t hsync_start;
  uint8_t hsync_pw;
  uint8_t hsync_finetune;
  uint8_t hsync_nondisp;
  uint8_t vsync_pw;
  uint16_t vsync_nondisp;
  uint16_t vsync_start;

  /* Set the correct values for the display being used */
  pixclk          = RA8875_PCSR_PDATL | RA8875_PCSR_2CLK;
  hsync_nondisp   = 26;
  hsync_start     = 32;
  hsync_pw        = 96;
  hsync_finetune  = 0;
  vsync_nondisp   = 32;
  vsync_start     = 23;
  vsync_pw        = 2;

  writeReg(RA8875_PCSR, pixclk);
  delay(1);

  /* Horizontal settings registers */
  writeReg(RA8875_HDWR, (_tft_width / 8) - 1);                          // H width: (HDWR + 1) * 8 = 480
  writeReg(RA8875_HNDFTR, RA8875_HNDFTR_DE_HIGH + hsync_finetune);
  writeReg(RA8875_HNDR, (hsync_nondisp - hsync_finetune - 2) / 8);  // H non-display: HNDR * 8 + HNDFTR + 2 = 10
  writeReg(RA8875_HSTR, hsync_start / 8 - 1);                       // Hsync start: (HSTR + 1)*8
  writeReg(RA8875_HPWR, RA8875_HPWR_LOW + (hsync_pw / 8 - 1));      // HSync pulse width = (HPWR+1) * 8

  /* Vertical settings registers */
  writeReg(RA8875_VDHR0, (uint16_t)(_tft_height - 1) & 0xFF);
  writeReg(RA8875_VDHR1, (uint16_t)(_tft_height - 1) >> 8);
  writeReg(RA8875_VNDR0, vsync_nondisp - 1);                        // V non-display period = VNDR + 1
  writeReg(RA8875_VNDR1, vsync_nondisp >> 8);
  writeReg(RA8875_VSTR0, vsync_start - 1);                          // Vsync start position = VSTR + 1
  writeReg(RA8875_VSTR1, vsync_start >> 8);
  writeReg(RA8875_VPWR, RA8875_VPWR_LOW + vsync_pw - 1);            // Vsync pulse width = VPWR + 1

  /* Set active window X */
  writeReg(RA8875_HSAW0, 0);                                        // horizontal start point
  writeReg(RA8875_HSAW1, 0);
  writeReg(RA8875_HEAW0, (uint16_t)(_tft_width - 1) & 0xFF);            // horizontal end point
  writeReg(RA8875_HEAW1, (uint16_t)(_tft_width - 1) >> 8);

  /* Set active window Y */
  writeReg(RA8875_VSAW0, 0);                                        // vertical start point
  writeReg(RA8875_VSAW1, 0);
  writeReg(RA8875_VEAW0, (uint16_t)(_tft_height - 1) & 0xFF);           // horizontal end point
  writeReg(RA8875_VEAW1, (uint16_t)(_tft_height - 1) >> 8);

  /* ToDo: Setup touch panel? */

  /* Clear the entire window */
  writeReg(RA8875_MCLR, RA8875_MCLR_START | RA8875_MCLR_FULL);
  delay(500);
  writeReg(RA8875_PWRR, RA8875_PWRR_NORMAL | RA8875_PWRR_DISPON);
  writeReg(RA8875_GPIOX, 1);
  writeReg(RA8875_P1CR, RA8875_P1CR_ENABLE | (RA8875_PWM_CLK_DIV1024 & 0xF));
  writeReg(RA8875_P1DCR, 255);
  IO.setFrequency(16000000);
}

void GxCTRL_RA8875S::setWindowAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
  _is_clipping = (x0 != 0) || (y0 != 0) || ( x1 != _tft_width - 1) || (y1 != _tft_height - 1);
  rotateWindow(x0, y0, x1, y1);
  writeReg16(RA8875_HSAW0, x0); // horizontal start point
  writeReg16(RA8875_HEAW0, x1); // horizontal end point
  writeReg16(RA8875_VSAW0, y0); // vertical start point
  writeReg16(RA8875_VEAW0, y1); // vertical end point
  writeReg16(RA8875_CURH0, x0); // horizontal input cursor
  writeReg16(RA8875_CURV0, y0); // vertical input cursor
  IO.startTransaction();
  IO.writeData(RA8875_CMDWRITE);
  IO.writeData(RA8875_MRWC);
  IO.writeCommand(RA8875_DATAWRITE);
}

void GxCTRL_RA8875S::clearWindowAddress()
{
  _is_clipping = false;
  writeReg16(RA8875_HSAW0, 0); // horizontal start point
  writeReg16(RA8875_HEAW0, _tft_width - 1); // horizontal end point
  writeReg16(RA8875_VSAW0, 0); // vertical start point
  writeReg16(RA8875_VEAW0, _tft_height - 1); // vertical end point
  writeReg16(RA8875_CURH0, 0); // horizontal input cursor
  writeReg16(RA8875_CURV0, 0); // vertical input cursor
}


void GxCTRL_RA8875S::setRotation(uint8_t r)
{
  _rotation = r;
  return;
  // did not work, or don't know how to
  switch (r & 3)
  {
    case 0:
      writeReg(RA8875_MWCR0, 0x0C);
      //writeReg(RA8875_DPCR, 0);
      break;
    case 1:
      writeReg(RA8875_MWCR0, 0);
      //writeReg(RA8875_DPCR, 4);
      break;
    case 2:
      writeReg(RA8875_MWCR0, 0);
      //writeReg(RA8875_DPCR, 6);
      break;
    case 3:
      writeReg(RA8875_MWCR0, 0);
      //writeReg(RA8875_DPCR, 8);
      break;
  }
  return;
}

void GxCTRL_RA8875S::rotatePoint(int16_t& x, int16_t& y)
{
  int16_t x0 = x;
  switch (_rotation)
  {
    case 0:
      x = y;
      y = _tft_height - x0 - 1;
      break;
    case 1: break;
    case 2:
      x = _tft_width - y - 1;
      y = x0;
      break;
    case 3:
      x = _tft_width - x - 1;
      y = _tft_height - y - 1;
      break;
  }
}

void GxCTRL_RA8875S::rotatePoint(uint16_t& x, uint16_t& y)
{
  uint16_t x0 = x;
  switch (_rotation)
  {
    case 0:
      x = y;
      y = _tft_height - x0 - 1;
      break;
    case 1: break;
    case 2:
      x = _tft_width - y - 1;
      y = x0;
      break;
    case 3:
      x = _tft_width - x - 1;
      y = _tft_height - y - 1;
      break;
  }
}

void GxCTRL_RA8875S::rotateWindow(int16_t& x0, int16_t& y0, int16_t& x1, int16_t& y1)
{
  switch (_rotation)
  {
    case 0:
      {
        int16_t tx0 = x0;
        int16_t tx1 = x1;
        x0 = y0;
        x1 = y1;
        y0 = _tft_height - tx1 - 1;
        y1 = _tft_height - tx0 - 1;
      }
      break;
    case 1:
      break;
    case 2:
      {
        int16_t tx0 = x0;
        int16_t tx1 = x1;
        x0 = _tft_width - y1 - 1;
        x1 = _tft_width - y0 - 1;
        y0 = tx0;
        y1 = tx1;
      }
      break;
    case 3:
      {
        int16_t tx0 = x0;
        int16_t ty0 = y0;
        x0 = _tft_width - x1 - 1;
        x1 = _tft_width - tx0 - 1;
        y0 = _tft_height - y1 - 1;
        y1 = _tft_height - ty0 - 1;
      }
      break;
  }
}

void GxCTRL_RA8875S::rotateWindow(uint16_t& x0, uint16_t& y0, uint16_t& x1, uint16_t& y1)
{
  switch (_rotation)
  {
    case 0:
      {
        uint16_t tx0 = x0;
        uint16_t tx1 = x1;
        x0 = y0;
        x1 = y1;
        y0 = _tft_height - tx1 - 1;
        y1 = _tft_height - tx0 - 1;
      }
      break;
    case 1:
      break;
    case 2:
      {
        uint16_t tx0 = x0;
        uint16_t tx1 = x1;
        x0 = _tft_width - y1 - 1;
        x1 = _tft_width - y0 - 1;
        y0 = tx0;
        y1 = tx1;
      }
      break;
    case 3:
      {
        uint16_t tx0 = x0;
        uint16_t ty0 = y0;
        x0 = _tft_width - x1 - 1;
        x1 = _tft_width - tx0 - 1;
        y0 = _tft_height - y1 - 1;
        y1 = _tft_height - ty0 - 1;
      }
      break;
  }
}

void GxCTRL_RA8875S::writeColor24(uint16_t color)
{
  writeReg(0x63, (color & 0xf800) >> 11);
  writeReg(0x64, (color & 0x07e0) >> 5);
  writeReg(0x65, (color & 0x001f));
}

void GxCTRL_RA8875S::drawPixel(uint16_t x, uint16_t y, uint16_t color)
{
  rotatePoint(x, y);
  writeReg16(RA8875_CURH0, x);
  writeReg16(RA8875_CURV0, y);
  writeCommand(RA8875_MRWC);
  IO.startTransaction();
  IO.writeCommand(RA8875_DATAWRITE);
  IO.writeData(color >> 8);
  IO.writeData(color);
  IO.endTransaction();
}

void GxCTRL_RA8875S::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
  if (_is_clipping) clearWindowAddress();
  rotatePoint(x0, y0);
  rotatePoint(x1, y1);
  writeReg16(0x91, x0); // Xs
  writeReg16(0x93, y0); // Ys
  writeReg16(0x95, x1); // Xe
  writeReg16(0x97, y1); // Ye
  writeColor24(color);
  writeReg(RA8875_DCR, 0x80); // draw
  /* Wait for the command to finish */
  waitPoll(RA8875_DCR, RA8875_DCR_LINESQUTRI_STATUS);
}

void GxCTRL_RA8875S::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  rectHelper(x, y, w, h, color, false);
}

void GxCTRL_RA8875S::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  rectHelper(x, y, w, h, color, true);
}

bool GxCTRL_RA8875S::drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
  return circleHelper(x0, y0, r, color, false);
}

bool GxCTRL_RA8875S::fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
  return circleHelper(x0, y0, r, color, true);
}

bool GxCTRL_RA8875S::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
  return triangleHelper(x0, y0, x1, y1, x2, y2, color, false);
}

bool GxCTRL_RA8875S::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
  return triangleHelper(x0, y0, x1, y1, x2, y2, color, true);
}

bool GxCTRL_RA8875S::drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t radius, uint16_t color)
{
  return roundRectHelper(x, y, w, h, radius, color, false);
}

bool GxCTRL_RA8875S::fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t radius, uint16_t color)
{
  return roundRectHelper(x, y, w, h, radius, color, false);
}

bool GxCTRL_RA8875S::drawEllipse(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint16_t color)
{
  return ellipseHelper(xCenter, yCenter, longAxis, shortAxis, color, false);
}

bool GxCTRL_RA8875S::fillEllipse(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint16_t color)
{
  return ellipseHelper(xCenter, yCenter, longAxis, shortAxis, color, true);
}

bool GxCTRL_RA8875S::drawCurve(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint8_t curvePart, uint16_t color)
{
  return curveHelper(xCenter, yCenter, longAxis, shortAxis, curvePart, color, false);
}

bool GxCTRL_RA8875S::fillCurve(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint8_t curvePart, uint16_t color)
{
  return curveHelper(xCenter, yCenter, longAxis, shortAxis, curvePart, color, true);
}

void GxCTRL_RA8875S::rectHelper(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color, bool filled)
{
  if (_is_clipping) clearWindowAddress();
  int16_t xe = x + w - 1;
  int16_t ye = y + h - 1;
  rotateWindow(x, y, xe, ye);
  writeReg16(0x91, x); // Xs
  writeReg16(0x93, y); // Ys
  writeReg16(0x95, xe); // Xe
  writeReg16(0x97, ye); // Ye
  writeColor24(color);
  writeReg(RA8875_DCR, filled ? 0xB0 : 0x90); // draw
  /* Wait for the command to finish */
  waitPoll(RA8875_DCR, RA8875_DCR_LINESQUTRI_STATUS);
}

bool GxCTRL_RA8875S::circleHelper(int16_t x0, int16_t y0, int16_t r, uint16_t color, bool filled)
{
  if (_is_clipping) clearWindowAddress();
  rotatePoint(x0, y0);
  writeReg16(0x99, x0);
  writeReg16(0x9b, y0);
  writeReg(0x9d, r);
  writeColor24(color);
  writeReg(RA8875_DCR, filled ? (RA8875_DCR_CIRCLE_START | RA8875_DCR_FILL) : (RA8875_DCR_CIRCLE_START | RA8875_DCR_NOFILL)); // draw
  /* Wait for the command to finish */
  IO.setFrequency(4000000);  // slow down for read, SUMOTOY's trick
  waitPoll(RA8875_DCR, RA8875_DCR_CIRCLE_STATUS);
  IO.setFrequency(16000000);
  return true;
}

bool GxCTRL_RA8875S::triangleHelper(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color, bool filled)
{
  if (_is_clipping) clearWindowAddress();
  rotatePoint(x0, y0);
  rotatePoint(x1, y1);
  rotatePoint(x2, y2);
  writeReg16(0x91, x0); // X0
  writeReg16(0x93, y0); // Y0
  writeReg16(0x95, x1); // X1
  writeReg16(0x97, y1); // Y1
  writeReg16(0xA9, x2); // X2
  writeReg16(0xAB, y2); // Y2
  writeColor24(color);
  writeReg(RA8875_DCR, filled ? 0xA1 : 0x81); // draw
  /* Wait for the command to finish */
  waitPoll(RA8875_DCR, RA8875_DCR_LINESQUTRI_STATUS);
  return true;
}

bool GxCTRL_RA8875S::roundRectHelper(int16_t x, int16_t y, int16_t w, int16_t h, int16_t radius, uint16_t color, bool filled)
{
  if (_is_clipping) clearWindowAddress();
  int16_t xe = x + w - 1;
  int16_t ye = y + h - 1;
  rotateWindow(x, y, xe, ye);
  writeReg16(0x91, x); // Xs
  writeReg16(0x93, y); // Ys
  writeReg16(0x95, xe); // Xe
  writeReg16(0x97, ye); // Ye
  writeReg16(0xA1, radius);
  writeReg16(0xA3, radius);
  writeColor24(color);
  writeReg(RA8875_ELLIPSE, filled ? 0xE0 : 0xA0); // draw
  /* Wait for the command to finish */
  waitPoll(RA8875_ELLIPSE, RA8875_DCR_LINESQUTRI_STATUS);
}

bool GxCTRL_RA8875S::ellipseHelper(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint16_t color, bool filled)
{
  if (_is_clipping) clearWindowAddress();
  rotatePoint(xCenter, yCenter);
  writeReg16(0xA5, xCenter);
  writeReg16(0xA7, yCenter);
  writeReg16(0xA1, longAxis);
  writeReg16(0xA3, shortAxis);
  writeColor24(color);
  writeReg(RA8875_ELLIPSE, filled ? 0xC0 : 0x80); // draw
  /* Wait for the command to finish */
  waitPoll(RA8875_ELLIPSE, RA8875_ELLIPSE_STATUS);
  return true;
}

bool GxCTRL_RA8875S::curveHelper(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint8_t curvePart, uint16_t color, bool filled)
{
  if (_is_clipping) clearWindowAddress();
  rotatePoint(xCenter, yCenter);
  writeReg16(0xA5, xCenter);
  writeReg16(0xA7, yCenter);
  writeReg16(0xA1, longAxis);
  writeReg16(0xA3, shortAxis);
  writeColor24(color);
  writeReg(RA8875_ELLIPSE, filled ? (0xD0 | (curvePart & 0x03)) : (0x90 | (curvePart & 0x03))); // draw
  /* Wait for the command to finish */
  waitPoll(RA8875_ELLIPSE, RA8875_ELLIPSE_STATUS);
  return true;
}

boolean GxCTRL_RA8875S::waitPoll(uint8_t regname, uint8_t waitflag)
{
  /* Wait for the command to finish */
  unsigned long timeout = millis();
  while (1)
  {
    uint8_t temp = readReg(regname);
    if (!(temp & waitflag)) return true;
    if ((millis() - timeout) > 20) return false;
  }
  return false;
}

void GxCTRL_RA8875S::writeReg(uint8_t reg, uint8_t val)
{
  writeCommand(reg);
  writeData(val);
}

void GxCTRL_RA8875S::writeReg16(uint8_t reg, uint16_t val)
{
  writeCommand(reg);
  writeData(val);
  writeCommand(reg + 1);
  writeData(val >> 8);
}

uint8_t GxCTRL_RA8875S::readReg(uint8_t reg)
{
  writeCommand(reg);
  return readData();
}

void GxCTRL_RA8875S::writeData(uint8_t d)
{
  IO.startTransaction();
  IO.writeData(RA8875_DATAWRITE);
  IO.writeData(d);
  IO.endTransaction();
}

uint8_t  GxCTRL_RA8875S::readData(void)
{
  IO.startTransaction();
  IO.writeData(RA8875_DATAREAD);
  uint8_t x = IO.readData();
  IO.endTransaction();
  return x;
}

void  GxCTRL_RA8875S::writeCommand(uint8_t d)
{
  IO.startTransaction();
  IO.writeData(RA8875_CMDWRITE);
  IO.writeData(d);
  IO.endTransaction();
}

uint8_t  GxCTRL_RA8875S::readStatus(void)
{
  IO.startTransaction();
  IO.writeData(RA8875_CMDREAD);
  uint8_t x = IO.readData();
  IO.endTransaction();
  return x;
}



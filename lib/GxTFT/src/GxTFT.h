/***************************************************
  Arduino TFT graphics library targetted at the
  Mega board, HX8357B/C and ILI9481 display drivers.

  This library has been derived from the Adafruit_GFX
  library and the associated driver library. See text
  at the end of this file.

  This is a standalone library that contains the
  hardware driver, the graphics functions and the
  proportional fonts.

  The larger fonts are Run Length Encoded to reduce
  their FLASH footprint and speed rendering.

 ****************************************************/

// modified by Jean-Marc Zingg to be the GxTFT class for the GxTFT library
// (the "complete" display class without using Adafruit_GFX for graphics)
// original source taken from https://github.com/Bodmer/TFT_HX8357
// with additions taken from https://github.com/Bodmer/TFT_eSPI

#ifndef _GxTFT_H_
#define _GxTFT_H_

#include "GxIO/GxIO.h"
#include "GxCTRL/GxCTRL.h"

// include helper file to find the fonts from my development path
// should be commented out or removed on release, but may get forgotten
//#include <GxTFT_Fonts.h>

#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
#if !defined(__AVR_ATmega328P__)
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
#define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:.
#define LOAD_FONT8  // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
#endif
#define LOAD_GFXFF  // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48

#define FF_HEIGHT '/'  // '/' character used to set free font height above the baseline
#define FF_BOTTOM 'y'  // 'y' character used to set free font height below baseline

#define TL_DATUM 0 // Top left (default)
#define TC_DATUM 1 // Top centre
#define TR_DATUM 2 // Top right
#define ML_DATUM 3 // Middle left
#define CL_DATUM 3 // Centre left, same as above
#define MC_DATUM 4 // Middle centre
#define CC_DATUM 4 // Centre centre, same as above
#define MR_DATUM 5 // Middle right
#define CR_DATUM 5 // Centre right, same as above
#define BL_DATUM 6 // Bottom left
#define BC_DATUM 7 // Bottom centre
#define BR_DATUM 8 // Bottom right
#define L_BASELINE  9 // Left character baseline (Line the 'A' character would sit on)
#define C_BASELINE 10 // Centre character baseline
#define R_BASELINE 11 // Right character baseline

// Color definitions
#define BLACK       0x0000      /*   0,   0,   0 */
#define NAVY        0x000F      /*   0,   0, 128 */
#define DARKGREEN   0x03E0      /*   0, 128,   0 */
#define DARKCYAN    0x03EF      /*   0, 128, 128 */
#define MAROON      0x7800      /* 128,   0,   0 */
#define PURPLE      0x780F      /* 128,   0, 128 */
#define OLIVE       0x7BE0      /* 128, 128,   0 */
#define LIGHTGREY   0xC618      /* 192, 192, 192 */
#define DARKGREY    0x7BEF      /* 128, 128, 128 */
#define BLUE        0x001F      /*   0,   0, 255 */
#define GREEN       0x07E0      /*   0, 255,   0 */
#define CYAN        0x07FF      /*   0, 255, 255 */
#define RED         0xF800      /* 255,   0,   0 */
#define MAGENTA     0xF81F      /* 255,   0, 255 */
#define YELLOW      0xFFE0      /* 255, 255,   0 */
#define WHITE       0xFFFF      /* 255, 255, 255 */
#define ORANGE      0xFD20      /* 255, 165,   0 */
#define GREENYELLOW 0xAFE5      /* 173, 255,  47 */
#define PINK        0xF81F

// New color definitions, used for all my TFT libraries
#define TFT_BLACK       0x0000
#define TFT_NAVY        0x000F
#define TFT_DARKGREEN   0x03E0
#define TFT_DARKCYAN    0x03EF
#define TFT_MAROON      0x7800
#define TFT_PURPLE      0x780F
#define TFT_OLIVE       0x7BE0
#define TFT_LIGHTGREY   0xC618
#define TFT_DARKGREY    0x7BEF
#define TFT_BLUE        0x001F
#define TFT_GREEN       0x07E0
#define TFT_CYAN        0x07FF
#define TFT_RED         0xF800
#define TFT_MAGENTA     0xF81F
#define TFT_YELLOW      0xFFE0
#define TFT_WHITE       0xFFFF
#define TFT_ORANGE      0xFD20
#define TFT_GREENYELLOW 0xAFE5
#define TFT_PINK        0xF81F

#ifdef LOAD_GLCD
#include "Fonts/glcdfont.c"
#endif

#ifdef LOAD_FONT2
#include "Fonts/Font16.h"
#endif

#ifdef LOAD_FONT4
#include "Fonts/Font32rle.h"
#define LOAD_RLE
#endif

#ifdef LOAD_FONT6
#include "Fonts/Font64rle.h"
#ifndef LOAD_RLE
#define LOAD_RLE
#endif
#endif

#ifdef LOAD_FONT7
#include "Fonts/Font7srle.h"
#ifndef LOAD_RLE
#define LOAD_RLE
#endif
#endif

#ifdef LOAD_FONT8
#include "Fonts/Font72rle.h"
#ifndef LOAD_RLE
#define LOAD_RLE
#endif
#endif

#if defined(__AVR)
#include <avr/pgmspace.h>
#endif
#include <Arduino.h>
#include <Print.h>

#include "Fonts/GFXFF/gfxfont.h"

#if 0
// These register enumerations are not all used, but kept for possible future use
#define HX8357D 0xD
#define HX8357B 0xB

#define HX8357_NOP     0x00
#define HX8357_SWRESET 0x01
#define HX8357_RDDID   0x04
#define HX8357_RDDST   0x09

#define HX8357_RDPOWMODE  0x0A
#define HX8357_RDMADCTL  0x0B
#define HX8357_RDCOLMOD  0x0C
#define HX8357_RDDIM  0x0D
#define HX8357_RDDSDR  0x0F

#define HX8357_SLPIN   0x10
#define HX8357_SLPOUT  0x11
#define HX8357B_PTLON   0x12
#define HX8357B_NORON   0x13

#define HX8357_INVOFF  0x20
#define HX8357_INVON   0x21
#define HX8357_DISPOFF 0x28
#define HX8357_DISPON  0x29

#define HX8357_CASET   0x2A
#define HX8357_PASET   0x2B
#define HX8357_RAMWR   0x2C
#define HX8357_RAMRD   0x2E

#define HX8357B_PTLAR   0x30
#define HX8357_TEON  0x35
#define HX8357_TEARLINE  0x44
#define HX8357_MADCTL  0x36
#define HX8357_COLMOD  0x3A

#define HX8357_SETOSC 0xB0
#define HX8357_SETPWR1 0xB1
#define HX8357B_SETDISPLAY 0xB2
#define HX8357_SETRGB 0xB3
#define HX8357D_SETCOM  0xB6

#define HX8357B_SETDISPMODE  0xB4
#define HX8357D_SETCYC  0xB4
#define HX8357B_SETOTP 0xB7
#define HX8357D_SETC 0xB9

#define HX8357B_SET_PANEL_DRIVING 0xC0
#define HX8357D_SETSTBA 0xC0
#define HX8357B_SETDGC  0xC1
#define HX8357B_SETID  0xC3
#define HX8357B_SETDDB  0xC4
#define HX8357B_SETDISPLAYFRAME 0xC5
#define HX8357B_GAMMASET 0xC8
#define HX8357B_SETCABC  0xC9
#define HX8357_SETPANEL  0xCC


#define HX8357B_SETPOWER 0xD0
#define HX8357B_SETVCOM 0xD1
#define HX8357B_SETPWRNORMAL 0xD2

#define HX8357B_RDID1   0xDA
#define HX8357B_RDID2   0xDB
#define HX8357B_RDID3   0xDC
#define HX8357B_RDID4   0xDD

#define HX8357D_SETGAMMA 0xE0

#define HX8357B_SETGAMMA 0xC8
#define HX8357B_SETPANELRELATED  0xE9

#endif

// This is a structure to conveniently hold infomation on the default fonts
// Stores pointer to font character image address table, width table and height

typedef struct {
  const uint8_t *chartbl;
  const uint8_t *widthtbl;
  uint8_t height;
  uint8_t baseline;
} fontinfo;

// Now fill the structure
const PROGMEM fontinfo fontdata [] = {
  { 0, 0, 0, 0 },

  // GLCD font (Font 1) does not have all parameters
  { 0, 0, 8, 7 },

#ifdef LOAD_FONT2
  { (const uint8_t *)chrtbl_f16, widtbl_f16, chr_hgt_f16, baseline_f16},
#else
  { 0, 0, 0, 0 },
#endif

  // Font 3 current unused
  { 0, 0, 0, 0 },

#ifdef LOAD_FONT4
  { (const uint8_t *)chrtbl_f32, widtbl_f32, chr_hgt_f32, baseline_f32},
#else
  { 0, 0, 0, 0 },
#endif

  // Font 5 current unused
  { 0, 0, 0, 0 },

#ifdef LOAD_FONT6
  { (const uint8_t *)chrtbl_f64, widtbl_f64, chr_hgt_f64, baseline_f64},
#else
  { 0, 0, 0, 0 },
#endif

#ifdef LOAD_FONT7
  { (const uint8_t *)chrtbl_f7s, widtbl_f7s, chr_hgt_f7s, baseline_f7s},
#else
  { 0, 0, 0, 0 },
#endif

#ifdef LOAD_FONT8
  { (const uint8_t *)chrtbl_f72, widtbl_f72, chr_hgt_f72, baseline_f72}
#else
  { 0, 0, 0, 0 }
#endif
};

// Class member functions and variables
class GxTFT : public Print
{
  public:
    GxTFT(GxIO& io, GxCTRL& controller, uint16_t w, uint16_t h);

    void     init(),

             drawPixel(uint16_t x, uint16_t y, uint16_t color),

             drawChar(int16_t x, int16_t y, uint8_t c, uint16_t color, uint16_t bg, uint8_t font),
             setWindow(int16_t x0, int16_t y0, int16_t x1, int16_t y1),

             pushColor(uint16_t color),
             pushColor(uint16_t color, uint16_t len),

             pushColors(uint16_t *data, uint8_t len),
             pushColors(uint8_t  *data, uint32_t len),

             fillScreen(uint16_t color),

             drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color),
             drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color),
             drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color),

             drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color),
             fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color),
             drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color),
             fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color),

             setRotation(uint8_t r),
             invertDisplay(boolean i),

             drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color),
             drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color),
             fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color),
             fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color),

             drawEllipse(int16_t x0, int16_t y0, int16_t rx, int16_t ry, uint16_t color),
             fillEllipse(int16_t x0, int16_t y0, int16_t rx, int16_t ry, uint16_t color),

             drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color),
             fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color),
             drawCurve(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint8_t curvePart, uint16_t color),
             fillCurve(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint8_t curvePart, uint16_t color),

             drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color),

             setCursor(int16_t x, int16_t y),
             setCursor(int16_t x, int16_t y, uint8_t font),
             setTextColor(uint16_t color),
             setTextColor(uint16_t fgcolor, uint16_t bgcolor),
             setTextSize(uint8_t size),
             setTextFont(uint8_t font),
             setTextWrap(boolean wrap),
             setTextDatum(uint8_t datum),
             setTextPadding(uint16_t x_width);

#ifdef LOAD_GFXFF
    void setFreeFont(const GFXfont *f = NULL);
    void setTextFont(const GFXfont *f = NULL);
#else
    void setFreeFont(uint8_t font);
#endif

    uint16_t readPixel(uint16_t x, uint16_t y);
    void     readRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t* data);
    void     writeRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *data);
    void     pushRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *data); // alias, name used in Bodmer's examples

    uint8_t  getRotation(void);

    uint16_t fontsLoaded(void),
             color565(uint8_t r, uint8_t g, uint8_t b);

    int16_t  drawChar(unsigned int uniCode, int x, int y, int font),
             drawChar(unsigned int uniCode, int x, int y),
             drawNumber(long long_num, int poX, int poY, int font),
             drawNumber(long long_num, int poX, int poY),
             drawFloat(float floatNumber, int decimal, int poX, int poY, int font),
             drawFloat(float floatNumber, int decimal, int poX, int poY),

             // Handle char arrays
             drawString(const char *string, int poX, int poY, int font),
             drawString(const char *string, int poX, int poY),

             // Handle String type
             drawString(const String& string, int poX, int poY, int font),
             drawString(const String& string, int poX, int poY);

  int16_t  height(void),
           width(void),
           textWidth(const char *string, int font),
           textWidth(const char *string),
           textWidth(const String& string, int font),
           textWidth(const String& string),
           fontHeight(int16_t font);

    virtual  size_t write(uint8_t c);

  private:
    // Sketches should use setWindow(...) instead of this one
    //void  setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

    uint8_t  tabcolor;

  protected:

    uint8_t   _initial_rotation;         // Display orientation as by constructor, kept for init()
    uint16_t  _tft_width, _tft_height;   // Display w/h in portrait orientation (default)
    uint16_t  _width, _height,           // Display w/h as modified by current rotation
              cursor_x, cursor_y, padX;  // Text cursor position and width padding

    uint16_t textcolor, textbgcolor, fontsloaded;

    uint8_t  glyph_ab,  // glyph height above baseline
             glyph_bb,  // glyph height below baseline
             textfont,  // Current selected font
             textsize,  // Current font size multiplier
             textdatum, // Text reference datum
             rotation;  // Display rotation (0-3)

    int8_t   _cs, _rs, _rst, _wr, _fcs; // Control lines

    boolean  textwrap; // If set, 'wrap' text at right edge of display

#ifdef LOAD_GFXFF
    GFXfont
    *gfxFont;
#endif

    GxIO& IO;
    GxCTRL& Controller;

};

#endif

/***************************************************

  ORIGINAL LIBRARY HEADER

  This is our library for the Adafruit  HX8357 Breakout and Shield
  ----> http://www.adafruit.com/products/1651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

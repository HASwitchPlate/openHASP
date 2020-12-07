/*
  See rights and use declaration in License.h
  This library has been modified for the Maple Mini
*/

// modified by Jean-Marc Zingg to be the GxTFT_GFX class for the GxTFT library
// (the "light" display class using Adafruit_GFX for graphics)
// original source taken from https://github.com/stevstrong/Adafruit_ILI9486_STM32/tree/master

#ifndef _GxTFT_GFX_
#define _GxTFT_GFX_

#include <Adafruit_GFX.h>
#include "GxIO/GxIO.h"
#include "GxCTRL/GxCTRL.h"

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

//#define swap(a, b) { int16_t t = a; a = b; b = t; }

/*****************************************************************************/
class GxTFT_GFX : public Adafruit_GFX
{
  public:

    GxTFT_GFX(GxIO& io, GxCTRL& controller, uint16_t w, uint16_t h);

    void	init(),
          setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1),
          pushColor(uint16_t color),
          fillScreen(uint16_t color),
          drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color),
          drawPixel(int16_t x, int16_t y, uint16_t color),
          drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color),
          drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color),
          fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color),
          setRotation(uint8_t r),
          invertDisplay(boolean i);
    uint16_t color565(uint8_t r, uint8_t g, uint8_t b);
    void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
    void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
    void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t radius, uint16_t color);
    void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t radius, uint16_t color);
    void drawEllipse(int16_t x0, int16_t y0, int16_t rx, int16_t ry, uint16_t color);
    void fillEllipse(int16_t x0, int16_t y0, int16_t rx, int16_t ry, uint16_t color);
    void drawCurve(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint8_t curvePart, uint16_t color);
    void fillCurve(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint8_t curvePart, uint16_t color);

    void reset(void);

  private:
    uint8_t	tabcolor;
    GxIO& IO;
    GxCTRL& Controller;
    uint8_t _initial_rotation;         // Display orientation as by constructor, kept for init()
    int16_t _tft_width, _tft_height;   // Display w/h in portrait orientation (default)
};

#endif //endif of the header file

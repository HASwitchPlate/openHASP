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

#ifndef _GxCTRL_RA8875S_H_
#define _GxCTRL_RA8875S_H_

#include "../GxCTRL.h"

class GxCTRL_RA8875S : public GxCTRL
{
  public:
    GxCTRL_RA8875S(GxIO& io) : GxCTRL(io), _tft_width(800), _tft_height(480) {};
    GxCTRL_RA8875S(GxIO& io, uint16_t tft_width, uint16_t tft_height) : GxCTRL(io), _tft_width(tft_width), _tft_height(tft_height) {};
    const char* name = "GxCTRL_RA8875S";
    const uint32_t ID = 0x8875;
    uint32_t readID();
    uint32_t readRegister(uint8_t nr, uint8_t index = 0, uint8_t bytes = 1);
    uint16_t readPixel(uint16_t x, uint16_t y);
    void     readRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t* data);
    void init();
    void setWindowAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
    void clearWindowAddress(); // reset clipping by controller
    void setRotation(uint8_t r);
    // optimization for this controller
    void drawPixel(uint16_t x, uint16_t y, uint16_t color);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    bool drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    bool fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    bool drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
    bool fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
    bool drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t radius, uint16_t color);
    bool fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t radius, uint16_t color);
    bool drawEllipse(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint16_t color);
    bool fillEllipse(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint16_t color);
    bool drawCurve(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint8_t curvePart, uint16_t color);
    bool fillCurve(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint8_t curvePart, uint16_t color);
  private:
    void rotatePoint(int16_t& x, int16_t& y);
    void rotatePoint(uint16_t& x, uint16_t& y);
    void rotateWindow(int16_t& x0, int16_t& y0, int16_t& x1, int16_t& y1);
    void rotateWindow(uint16_t& x0, uint16_t& y0, uint16_t& x1, uint16_t& y1);
    void writeColor24(uint16_t color);
    void rectHelper  (int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color, bool filled);
    bool circleHelper(int16_t x0, int16_t y0, int16_t r, uint16_t color, bool filled);
    bool triangleHelper(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color, bool filled);
    bool roundRectHelper(int16_t x, int16_t y, int16_t w, int16_t h, int16_t radius, uint16_t color, bool filled);
    bool ellipseHelper(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint16_t color, bool filled);
    bool curveHelper(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint8_t curvePart, uint16_t color, bool filled);
    void    writeReg(uint8_t reg, uint8_t val);
    void    writeReg16(uint8_t reg, uint16_t val);
    uint8_t readReg(uint8_t reg);
    void    writeData(uint8_t d);
    uint8_t readData(void);
    void    writeCommand(uint8_t d);
    uint8_t readStatus(void);
    boolean waitPoll(uint8_t r, uint8_t f);
    uint8_t _rotation;
    uint16_t _tft_width, _tft_height; // physical
    bool _is_clipping;
};

#define GxCTRL_Class GxCTRL_RA8875S

#endif


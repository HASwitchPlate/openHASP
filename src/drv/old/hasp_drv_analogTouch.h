// Touch screen library with X Y and Z (pressure) readings as well
// as oversampling to avoid 'bouncing'
// (c) ladyada / adafruit
// Code under MIT License

#ifndef _ADAFRUIT_TOUCHSCREEN_H_
#define _ADAFRUIT_TOUCHSCREEN_H_

#include <stdint.h>

#ifdef ARDUINO
#include <Arduino.h>

#define ADC_MAX 4095   // maximum value for ESP32 ADC (default 11db, 12 bits)
#define aXM TOUCH_anDC // analog input pin connected to LCD_RS
#define aYP TOUCH_anWR // analog input pin connected to LCD_WR

#define NOISE_LEVEL 4 // Allow small amount of measurement noise

typedef volatile uint32_t RwReg;

class TSPoint {
  public:
    TSPoint(void);
    TSPoint(int16_t x, int16_t y, int16_t z);

    bool operator==(TSPoint);
    bool operator!=(TSPoint);

    int16_t x, y, z;
};

class TouchScreen {
  public:
    TouchScreen(uint8_t xp, uint8_t yp, uint8_t xm, uint8_t ym, uint16_t rx);

    TSPoint getPoint();

  private:
    uint8_t _yp, _ym, _xm, _xp;
    uint16_t _rxplate;

    volatile RwReg *xp_port, *yp_port, *xm_port, *ym_port;
    RwReg xp_pin, xm_pin, yp_pin, ym_pin;
};

// increase or decrease the touchscreen oversampling. This is a little different than you make think:
// 1 is no oversampling, whatever data we get is immediately returned
// 2 is double-sampling and we only return valid data if both points are the same
// 3+ uses insert sort to get the median value.
// We found 2 is precise yet not too slow so we suggest sticking with it!

#define NUMSAMPLES 2

TSPoint::TSPoint(void)
{
    x = y = 0;
}

TSPoint::TSPoint(int16_t x0, int16_t y0, int16_t z0)
{
    x = x0;
    y = y0;
    z = z0;
}

bool TSPoint::operator==(TSPoint p1)
{
    return ((p1.x == x) && (p1.y == y) && (p1.z == z));
}

bool TSPoint::operator!=(TSPoint p1)
{
    return ((p1.x != x) || (p1.y != y) || (p1.z != z));
}

TSPoint TouchScreen::getPoint(void)
{
    int x, y, z;
    int samples[NUMSAMPLES];
    uint8_t i;

    pinMode(_yp, INPUT);
    pinMode(_ym, INPUT);
    pinMode(_xp, OUTPUT);
    pinMode(_xm, OUTPUT);

    digitalWrite(_xp, HIGH);
    digitalWrite(_xm, LOW);

    for(i = 0; i < NUMSAMPLES; i++) {
        samples[i] = analogRead(aYP);
    }

    samples[1] = (samples[0] + samples[1]) >> 1; // average 2 samples
    x          = (ADC_MAX - samples[NUMSAMPLES / 2]);

    pinMode(_xp, INPUT);
    pinMode(_xm, INPUT);
    pinMode(_yp, OUTPUT);
    pinMode(_ym, OUTPUT);

    digitalWrite(_ym, LOW);
    digitalWrite(_yp, HIGH);

    for(i = 0; i < NUMSAMPLES; i++) {
        samples[i] = analogRead(aXM);
    }

    samples[1] = (samples[0] + samples[1]) >> 1; // average 2 samples

    y = (ADC_MAX - samples[NUMSAMPLES / 2]);

    // Set X+ to ground
    // Set Y- to VCC
    // Hi-Z X- and Y+
    pinMode(_xp, OUTPUT);
    pinMode(_yp, INPUT);

    digitalWrite(_xp, LOW);
    digitalWrite(_ym, HIGH);

    int z1 = analogRead(aXM);
    int z2 = analogRead(aYP);

    z = (ADC_MAX - (z2 - z1));

    // Restore pin states
    pinMode(_xm, OUTPUT); 
    pinMode(_yp, OUTPUT);
    return TSPoint(x, y, z);
}

TouchScreen::TouchScreen(uint8_t xp, uint8_t yp, uint8_t xm, uint8_t ym, uint16_t rxplate = 0)
{
    _yp      = yp;
    _xm      = xm;
    _ym      = ym;
    _xp      = xp;
    _rxplate = rxplate;

}

#endif // ARDUINO
#endif // _ADAFRUIT_TOUCHSCREEN_H_
// Touch screen library with X Y and Z (pressure) readings as well
// as oversampling to avoid 'bouncing'
// (c) ladyada / adafruit
// Code under MIT License

#ifndef _ADAFRUIT_TOUCHSCREEN_H_
#define _ADAFRUIT_TOUCHSCREEN_H_
#include <stdint.h>

// ESP32 specific
#define ESP32_WIFI_TOUCH // uncomment to use parallel MCU Friend LCD touchscreen with ESP32 UNO Wifi
#ifdef ESP32
#define ADC_MAX 4095 // maximum value for ESP32 ADC (default 11db, 12 bits)
#define aXM 35       // analog input pin connected to LCD_RS
#define aYP 39       // analog input pin connected to LCD_WR
#else
#define ADC_MAX 1023 // Arduino
#endif
#define NOISE_LEVEL 4 // Allow small amount of measurement noise

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega32U4__) || defined(TEENSYDUINO) || defined(__AVR_ATmega2560__)
typedef volatile uint8_t RwReg;
#endif
#if defined(ARDUINO_STM32_FEATHER)
typedef volatile uint32 RwReg;
#endif
#if defined(ARDUINO_FEATHER52) || defined(ESP32)
typedef volatile uint32_t RwReg;
#endif

#if defined(__AVR__) || defined(TEENSYDUINO) || defined(ARDUINO_ARCH_SAMD)
#define USE_FAST_PINIO
#endif

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

    bool isTouching(void);
    uint16_t pressure(void);
    int readTouchY();
    int readTouchX();
    TSPoint getPoint();
    int16_t pressureThreshhold;

  private:
    uint8_t _yp, _ym, _xm, _xp;
    uint16_t _rxplate;

    volatile RwReg *xp_port, *yp_port, *xm_port, *ym_port;
    RwReg xp_pin, xm_pin, yp_pin, ym_pin;
};

#endif

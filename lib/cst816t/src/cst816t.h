#ifndef _CST816T_H_
#define _CST816T_H_

#include <Arduino.h>
#include <Wire.h>
#include <stdint.h>

#define CHIPID_CST716 0x20
#define CHIPID_CST816S 0xB4
#define CHIPID_CST816T 0xB5
#define CHIPID_CST816D 0xB6

#define GESTURE_NONE 0x00
#define GESTURE_SWIPE_UP 0x01
#define GESTURE_SWIPE_DOWN 0x02
#define GESTURE_SWIPE_LEFT 0x03
#define GESTURE_SWIPE_RIGHT 0x04
#define GESTURE_SINGLE_CLICK 0x05
#define GESTURE_DOUBLE_CLICK 0x0B
#define GESTURE_LONG_PRESS 0x0C

enum touchpad_mode { 
  mode_touch,
  mode_change,
  mode_fast,
  mode_motion 
};

class cst816t {
public:
  cst816t(TwoWire &_wire, int8_t _rst, int8_t _irq);
  /*
     mode_touch: interrupt every 10ms when a touch is detected
     mode_change: interrupt when a touch change is detected
     mode_fast: interrupt when a click or swipe is detected
     mode_motion: interrupt when a click, swipe, double click, or long press is detected
  */
  bool begin(touchpad_mode tp_mode = mode_change);
  bool available();
  String version();
  String state();
  uint8_t chip_id;
  uint8_t firmware_version;
  uint8_t gesture_id;
  uint8_t finger_num;
  uint16_t x;
  uint16_t y;
private:
  TwoWire& wire;
  int8_t rst;
  int8_t irq;
  uint8_t i2c_read(uint16_t addr, uint8_t reg_addr, uint8_t *reg_data, uint32_t length);
  uint8_t i2c_write(uint8_t addr, uint8_t reg_addr, const uint8_t *reg_data, uint32_t length);
};

#endif

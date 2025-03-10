/* CST816T capacitive touch-screen driver.
 Koen De Vleeschauwer, 2023
 Literature:
 DS-CST816T-V2.2.pdf datasheet
 AN-CST816T-v1.pdf register description
 CST78XX-V1.0.pdf firmware upgrade how-to
*/

#include "cst816t.h"
#include <stdint.h>

#define CST816T_ADDRESS 0x15

#define REG_GESTURE_ID 0x01
#define REG_FINGER_NUM 0x02
#define REG_XPOS_H 0x03
#define REG_XPOS_L 0x04
#define REG_YPOS_H 0x05
#define REG_YPOS_L 0x06
#define REG_CHIP_ID 0xA7
#define REG_PROJ_ID 0xA8
#define REG_FW_VERSION 0xA9
#define REG_FACTORY_ID 0xAA
#define REG_SLEEP_MODE 0xE5
#define REG_IRQ_CTL 0xFA
#define REG_LONG_PRESS_TICK 0xEB
#define REG_MOTION_MASK 0xEC
#define REG_DIS_AUTOSLEEP 0xFE

#define MOTION_MASK_CONTINUOUS_LEFT_RIGHT 0b100
#define MOTION_MASK_CONTINUOUS_UP_DOWN 0b010
#define MOTION_MASK_DOUBLE_CLICK 0b001

#define IRQ_EN_TOUCH 0x40
#define IRQ_EN_CHANGE 0x20
#define IRQ_EN_MOTION 0x10
#define IRQ_EN_LONGPRESS 0x01

static bool tp_event = false;
static void tp_isr() {
  tp_event = true;
}

cst816t::cst816t(TwoWire &_wire, int8_t _rst, int8_t _irq)
  : wire(_wire) {
  rst = _rst;
  irq = _irq;
  chip_id = 0;
  firmware_version = 0;
  tp_event = false;
  gesture_id = 0;
  finger_num = 0;
  x = 0;
  y = 0;
}

bool cst816t::begin(touchpad_mode tp_mode) {
  uint8_t dta[4];
  pinMode(irq, INPUT);
  
  if (rst >= 0) {
    pinMode(rst, OUTPUT);
    digitalWrite(rst, HIGH);
    delay(100);
    digitalWrite(rst, LOW);
    delay(10);
    digitalWrite(rst, HIGH);
    delay(100);
  }

  wire.begin();
  
  if (i2c_read(CST816T_ADDRESS, REG_CHIP_ID, dta, sizeof(dta))) {
    Serial.println("i2c error");
    return false;  // Sikertelen inicializálás
  }

  chip_id = dta[0];
  firmware_version = dta[3];

  uint8_t irq_en = 0x0;
  uint8_t motion_mask = 0x0;
  switch (tp_mode) {
    case mode_touch:
      irq_en = IRQ_EN_TOUCH;
      break;
    case mode_change:
      irq_en = IRQ_EN_CHANGE;
      break;
    case mode_fast:
      irq_en = IRQ_EN_MOTION;
      break;
    case mode_motion:
      irq_en = IRQ_EN_MOTION | IRQ_EN_LONGPRESS;
      motion_mask = MOTION_MASK_DOUBLE_CLICK;
      break;
    default:
      return false; // Érvénytelen mód esetén sikertelen inicializálás
  }

  if (i2c_write(CST816T_ADDRESS, REG_IRQ_CTL, &irq_en, 1) ||
      i2c_write(CST816T_ADDRESS, REG_MOTION_MASK, &motion_mask, 1)) {
    Serial.println("i2c write error");
    return false;
  }

  attachInterrupt(digitalPinToInterrupt(irq), tp_isr, FALLING);

  // Disable auto sleep
  uint8_t dis_auto_sleep = 0xFF;
  if (i2c_write(CST816T_ADDRESS, REG_DIS_AUTOSLEEP, &dis_auto_sleep, 1)) {
    Serial.println("Failed to disable auto sleep");
    return false;
  }

  return true; // Sikeres inicializálás
}


bool cst816t::available() {
  uint8_t dta[6];
  if (tp_event && !i2c_read(CST816T_ADDRESS, REG_GESTURE_ID, dta, sizeof(dta))) {
    tp_event = false;
    gesture_id = dta[0];
    finger_num = dta[1];
    x = (((uint16_t)dta[2] & 0x0f) << 8) | (uint16_t)dta[3];
    y = (((uint16_t)dta[4] & 0x0f) << 8) | (uint16_t)dta[5];
    return true;
  }
  return false;
}

uint8_t cst816t::i2c_read(uint16_t addr, uint8_t reg_addr, uint8_t *reg_data, uint32_t length) {
  wire.beginTransmission(addr);
  wire.write(reg_addr);
  if (wire.endTransmission(true)) return -1;
  wire.requestFrom((uint8_t)addr, (size_t)length, (bool)true);
  for (int i = 0; i < length; i++) {
    *reg_data++ = wire.read();
  }
  return 0;
}

uint8_t cst816t::i2c_write(uint8_t addr, uint8_t reg_addr, const uint8_t *reg_data, uint32_t length) {
  wire.beginTransmission(addr);
  wire.write(reg_addr);
  for (int i = 0; i < length; i++) {
    wire.write(*reg_data++);
  }
  if (wire.endTransmission(true)) return -1;
  return 0;
}

String cst816t::version() {
  String tp_version;
  switch (chip_id) {
    case CHIPID_CST716: tp_version = "CST716"; break;
    case CHIPID_CST816S: tp_version = "CST816S"; break;
    case CHIPID_CST816T: tp_version = "CST816T"; break;
    case CHIPID_CST816D: tp_version = "CST816D"; break;
    default:
      tp_version = "? 0x" + String(chip_id, HEX);
      break;
  }
  tp_version += " firmware " + String(firmware_version);
  return tp_version;
}

String cst816t::state() {
  String s;
  switch (gesture_id) {
    case GESTURE_NONE:
      s = "NONE";
      break;
    case GESTURE_SWIPE_DOWN:
      s = "SWIPE DOWN";
      break;
    case GESTURE_SWIPE_UP:
      s = "SWIPE UP";
      break;
    case GESTURE_SWIPE_LEFT:
      s = "SWIPE LEFT";
      break;
    case GESTURE_SWIPE_RIGHT:
      s = "SWIPE RIGHT";
      break;
    case GESTURE_SINGLE_CLICK:
      s = "SINGLE CLICK";
      break;
    case GESTURE_DOUBLE_CLICK:
      s = "DOUBLE CLICK";
      break;
    case GESTURE_LONG_PRESS:
      s = "LONG PRESS";
      break;
    default:
      s = "UNKNOWN 0x";
      s += String(gesture_id, HEX);
      break;
  }
  s += '\t' + String(x) + '\t' + String(y);
  return s;
}

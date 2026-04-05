#ifndef _Arduino_TCA9554SWSPI_H_
#define _Arduino_TCA9554SWSPI_H_

#if defined(HASP_USE_ARDUINOGFX)
#warning Arduino_TCA9554SWSPI.h

#include <Wire.h>
#include <Arduino_GFX_Library.h>
#include "Arduino_DataBus.h"

#define TCA9554_IIC_ADDRESS 0x20

#define TCA9554_INPUT_PORT_REG      0x00
#define TCA9554_OUTPUT_PORT_REG     0x01
#define TCA9554_INVERSION_PORT_REG  0x02
#define TCA9554_CONFIG_PORT_REG     0x03

// Verified from stock trace
#define PIN_TCA_BL    0
#define PIN_TCA_CS    1
#define PIN_TCA_CLK   2
#define PIN_TCA_SDIO  3
#define MASK_POWER_ON 0xF0

#define BIT_TCA_BL    (1U << PIN_TCA_BL)
#define BIT_TCA_CS    (1U << PIN_TCA_CS)
#define BIT_TCA_CLK   (1U << PIN_TCA_CLK)
#define BIT_TCA_SDIO  (1U << PIN_TCA_SDIO)

// Pin definitions on the TCA9554
#define BIT_MOSI (1 << 3)
#define BIT_CLK  (1 << 2)
#define BIT_EN   (1 << 1)

#define TCA9554_IDLE_STATE (MASK_POWER_ON | BIT_TCA_CS)

static uint8_t _current_pins = 0;

class Arduino_TCA9554SWSPI : public Arduino_DataBus
{
public:
  Arduino_TCA9554SWSPI(int8_t i2c_sda, int8_t i2c_scl, int8_t spi_sdio, int8_t spi_sclk, int8_t spi_cs, TwoWire *wire = &Wire);

  bool begin(int32_t speed = GFX_NOT_DEFINED, int8_t dataMode = GFX_NOT_DEFINED) override;

  void _send_9bit(bool is_data, uint8_t value);

  void _update_tca(uint8_t state);

  void _pulse_clock();

  void beginWrite() override;
  void endWrite() override;

  void writeCommand(uint8_t c) override;
  void writeCommand16(uint16_t c) override;

  void write(uint8_t d) override;
  void write16(uint16_t d) override;

  void writeCommandBytes(uint8_t *data, uint32_t len) override;
  void writeBytes(uint8_t *data, uint32_t len) override;
#if defined(USE_I2C_SW_SPI)
  void writeRegisterI2COpcode(uint8_t reg, uint8_t data) override;
  void writeRegisterI2CSeqOpcode(uint8_t reg, const uint8_t *data, size_t len) override;
#endif
  void closeOut();
  void lcd_power_and_brightness(uint8_t level);
  void initMotionSensor();
  void initMotionSensorRegistor();

protected:
  void writeRegister(uint8_t reg, const uint8_t *data, size_t len);
  void writeRegisterI2C(uint8_t reg, uint8_t data);
  uint8_t readRegister(uint8_t reg, uint8_t *data, size_t len);

  uint8_t _address;
  bool is_found;

  int8_t _i2c_sda, _i2c_scl, _spi_sdio, _spi_sclk, _spi_cs;
  TwoWire *_wire;

private:
  uint8_t _output_cache;

  void flushOutput();
  void send_spi_bit(bool bit_val);
  void write9(bool is_data, uint8_t byte);

public:
  // Required by Arduino_DataBus, intentionally unused for this bus.
  void writeRepeat(uint16_t, uint32_t) override {}
  void writePixels(uint16_t *, uint32_t) override {}
};

#endif

#endif // _Arduino_TCA9554SWSPI_H_
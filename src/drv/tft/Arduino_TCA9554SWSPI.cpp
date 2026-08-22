#include "Arduino_TCA9554SWSPI.h"

#if defined(HASP_USE_ARDUINOGFX)

#include "driver/gpio.h"
#include "hasplib.h"

Arduino_TCA9554SWSPI::Arduino_TCA9554SWSPI(int8_t i2c_sda, int8_t i2c_scl, int8_t spi_sdio, int8_t spi_sclk, int8_t spi_cs, TwoWire *wire)
    : _address(TCA9554_IIC_ADDRESS),
      is_found(false),
      _i2c_sda(i2c_sda),
      _i2c_scl(i2c_scl),
      _spi_sdio(spi_sdio),
      _spi_sclk(spi_sclk),
      _spi_cs(spi_cs),
      _wire(wire),
      _output_cache(TCA9554_IDLE_STATE)
{
}

bool Arduino_TCA9554SWSPI::begin(int32_t speed, int8_t dataMode) {
  _wire->begin(_i2c_sda, _i2c_scl);

  // 1. [03 FF] -> [01 FF] Reset Jolt
  uint8_t val_ff = 0xFF;
  writeRegister(0x03, &val_ff, 1); 
  writeRegister(0x01, &val_ff, 1);
  delay(5); 

  // 2. [03 F1] Handshake Config
  uint8_t val_f1 = 0xF1;
  writeRegister(0x03, &val_f1, 1);
  delayMicroseconds(55);

  // 3. [01 FB] -> [01 F3] Power Enable
  uint8_t val_fb = 0xFB;
  writeRegister(0x01, &val_fb, 1);
  delayMicroseconds(50);
  uint8_t val_f3 = 0xF3;
  writeRegister(0x01, &val_f3, 1);

  delay(55);

  // 4. THE MAGIC SYNC [00 00]
  // Manual I2C Write to Register 0x00 to avoid syntax errors
  _wire->beginTransmission(_address);
  _wire->write(0x00); // Register Pointer
  _wire->write(0x00); // Data Value
  _wire->endTransmission();

  delay(120); 

  is_found = true;
  return true;
}

// Internal helper to update the TCA9554 pins over I2C
void Arduino_TCA9554SWSPI::_update_tca(uint8_t state) {
  Wire.beginTransmission(TCA9554_IIC_ADDRESS);
  Wire.write(TCA9554_OUTPUT_PORT_REG);
  Wire.write(state);
  Wire.endTransmission(true);
}

// The core dispatcher that aligns everything
void Arduino_TCA9554SWSPI::_send_9bit(bool is_data, uint8_t value) {
  // 1. Prepare 9th bit
  if (is_data) _current_pins |= BIT_MOSI;
  else         _current_pins &= ~BIT_MOSI;

  // Pulse Clock (3 I2C writes)
  _update_tca(_current_pins & ~BIT_CLK);
  _update_tca(_current_pins | BIT_CLK);
  _update_tca(_current_pins & ~BIT_CLK);

  // 2. Data Bits
  for (int i = 7; i >= 0; i--) {
      if ((value >> i) & 0x01) _current_pins |= BIT_MOSI;
      else                     _current_pins &= ~BIT_MOSI;

      // Optimized Clock Pulse
      _update_tca(_current_pins & ~BIT_CLK);
      _update_tca(_current_pins | BIT_CLK);
      _update_tca(_current_pins & ~BIT_CLK);
  }

  // 3. Latch (Yellow Pulse)
  _update_tca(_current_pins | BIT_EN);
  _update_tca(_current_pins & ~BIT_EN);
}


// Internal helper to pulse the clock
void Arduino_TCA9554SWSPI::_pulse_clock() {
  _update_tca(_current_pins & ~BIT_CLK); // CLK Low
  _update_tca(_current_pins | BIT_CLK);  // CLK High (Rising Edge)
  _update_tca(_current_pins & ~BIT_CLK); // CLK Low
}

void Arduino_TCA9554SWSPI::write9(bool is_data, uint8_t byte) {
  _send_9bit(is_data, byte);
}

void Arduino_TCA9554SWSPI::write(uint8_t d)
{
  _send_9bit(true, d);
}

void Arduino_TCA9554SWSPI::beginWrite()
{
}

void Arduino_TCA9554SWSPI::endWrite()
{
}

void Arduino_TCA9554SWSPI::writeCommand(uint8_t c)
{
  _send_9bit(false, c);
}

void Arduino_TCA9554SWSPI::writeCommand16(uint16_t c)
{
  writeCommand((uint8_t)(c >> 8));
  writeCommand((uint8_t)(c & 0xFF));
}

void Arduino_TCA9554SWSPI::write16(uint16_t d)
{
  write((uint8_t)(d >> 8));
  write((uint8_t)(d & 0xFF));
}

void Arduino_TCA9554SWSPI::writeCommandBytes(uint8_t *data, uint32_t len)
{
  if (!data || !len) return;

  beginWrite();
  writeCommand(data[0]);
  for (uint32_t i = 1; i < len; ++i) {
    write(data[i]);
  }
  endWrite();
}

void Arduino_TCA9554SWSPI::writeBytes(uint8_t *data, uint32_t len)
{
  if (!data || !len) return;

  beginWrite();
  for (uint32_t i = 0; i < len; ++i) {
    write(data[i]);
  }
  endWrite();
}

// Exposed through Arduino_DataBus when USE_I2C_SW_SPI is enabled.
#if defined(USE_I2C_SW_SPI)
void Arduino_TCA9554SWSPI::writeRegisterI2COpcode(uint8_t reg, uint8_t data)
{
  writeRegisterI2C(reg, data);
}

void Arduino_TCA9554SWSPI::writeRegisterI2CSeqOpcode(uint8_t reg, const uint8_t *data, size_t len)
{
  writeRegister(reg, data, len);
}
#endif

// Custom Methods

void Arduino_TCA9554SWSPI::flushOutput()
{
  if (!is_found) return;

  uint8_t out = (_output_cache | MASK_POWER_ON);
  writeRegister(TCA9554_OUTPUT_PORT_REG, &out, 1);
}

void Arduino_TCA9554SWSPI::closeOut() {
  // --- Phase 1: SPI Wake & Calibration ---
  
  // 11 - Sleep Out
  beginWrite();
  write9(false, 0x11);
  endWrite();
  delay(121);

  // 29 - Display ON
  beginWrite();
  write9(false, 0x29);
  endWrite();
  delay(22); // The 'delay 29' from stock

  // 29 - Display ON
  beginWrite();
  write9(false, 0x29);
  endWrite();
  delay(2); // The 'delay 29' from stock

  // B1 13 - First Tap
  beginWrite();
  write9(false, 0xB1);
  write9(true, 0x13);
  endWrite();
  delay(40); // The 'delay 35' from stock

  // B1 13 - Second Tap
  beginWrite();
  write9(false, 0xB1);
  write9(true, 0x13);
  endWrite();

  // --- Phase 2: Hardware Cool Down (The Hand-off) ---

  // 01 F1 - MOSI Low
  // uint8_t mosi_low = 0xF1;
  // writeRegister(0x01, &mosi_low, 1);

  // 01 F3 - CS High
  // uint8_t cs_high = 0xF3;
  // writeRegister(0x01, &cs_high, 1);

  writeRegisterI2C(0x01, 0xB1);
  delay(435);
  writeRegisterI2C(0x01, 0xB1);
  delay(435);
  
  writeRegisterI2C(0x01, 0xF1);
  writeRegisterI2C(0x01, 0xF3);

  // 2F 00 - 19 Times
  for(int i = 0; i < 19; i++) {
      writeRegisterI2C(0x2F, 0x00);
  }
}

void Arduino_TCA9554SWSPI::lcd_power_and_brightness(uint8_t level) {

  // 2. Set Brightness (Verified 6-byte sequence from your CSV)
  // Address 0x20, Command 0x21, Sub 0x01, Level, 0x00, 0x00, 0x14
  if (level > 100) level = 100;

  LOG_INFO(TAG_TFT, F("Lanbon L10 brightness set to %u"), level);
  
  Wire.beginTransmission(0x20);
  Wire.write(0x21); 
  Wire.write(0x01); 
  Wire.write(level); 
  Wire.write(0x00);
  Wire.write(0x00);
  Wire.write(0x14); 
  Wire.endTransmission();
}


// 0x27·0x00·0x01·0x01·0x00·0x00·0x0B
void Arduino_TCA9554SWSPI::initMotionSensorRegistor() {
  Wire.beginTransmission(TCA9554_IIC_ADDRESS);  
  Wire.write(0x27);
  Wire.write(0x00);
  Wire.write(0x01);
  Wire.write(0x01);
  Wire.write(0x00);
  Wire.write(0x00);
  Wire.write(0x0B);
  Wire.endTransmission();
}

// W[0x20]·0x1B·0x90·0x80·0x00·0x00·0x00·0x0D·0xAE·0x33·0xAF·0x8D·0xAE·0x33·0xAF·
// 0x80·0x00·0x00·0x00·0x00·0x01·0x80·0x00
void Arduino_TCA9554SWSPI::initMotionSensor() {
  Wire.beginTransmission(TCA9554_IIC_ADDRESS);  
  Wire.write(0x1B);
  Wire.write(0x90);
  Wire.write(0x80);
  Wire.write(0x00);
  Wire.write(0x00);
  Wire.write(0x00);
  Wire.write(0x0D);
  Wire.write(0xAE);
  Wire.write(0x33);
  Wire.write(0xAF);
  Wire.write(0x8D);
  Wire.write(0xAE);
  Wire.write(0x33);
  Wire.write(0xAF);
  Wire.write(0x80);
  Wire.write(0x00);
  Wire.write(0x00); 
  Wire.write(0x00);
  Wire.write(0x00); 
  Wire.write(0x01);
  Wire.write(0x80);
  Wire.write(0x00);
  Wire.endTransmission();
}

void Arduino_TCA9554SWSPI::writeRegister(uint8_t reg, const uint8_t *data, size_t len)
{
  _wire->beginTransmission(_address);
  _wire->write(reg);
  for (size_t i = 0; i < len; ++i) {
    _wire->write(data[i]);
  }
  
  _wire->endTransmission();
}

void Arduino_TCA9554SWSPI::writeRegisterI2C(uint8_t reg,  uint8_t data)
{
  _wire->beginTransmission(_address);
  _wire->write(reg);
  _wire->write(data);  
  _wire->endTransmission();
}

uint8_t Arduino_TCA9554SWSPI::readRegister(uint8_t reg, uint8_t *data, size_t len)
{
  _wire->beginTransmission(_address);
  _wire->write(reg);
  if (_wire->endTransmission(false) != 0) {
    return 0;
  }

  size_t got = _wire->requestFrom((int)_address, (int)len);
  for (size_t i = 0; i < got; ++i) {
    data[i] = _wire->read();
  }
  return (uint8_t)got;
}

#endif
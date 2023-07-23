#include "Arduino_PCA9535SWSPI.h"

#if defined(HASP_USE_ARDUINOGFX)
#warning Arduino_PCA9535SWSPI.cpp

#include "driver/gpio.h"
#include "hasplib.h"

Arduino_PCA9535SWSPI::Arduino_PCA9535SWSPI(int8_t sda, int8_t scl, int8_t pwd, int8_t cs, int8_t sck, int8_t mosi, TwoWire *wire)
    : _sda(sda), _scl(scl), _pwd(pwd), _cs(cs), _sck(sck), _mosi(mosi), _wire(wire)
{
}

bool Arduino_PCA9535SWSPI::begin(int32_t speed, int8_t dataMode)
{
  UNUSED(speed);
  UNUSED(dataMode);

  _address = PCA9535_IIC_ADDRESS;
  _wire->beginTransmission(_address);
  if (!_wire->endTransmission())
  {
    LOG_INFO(TAG_DRVR, "Found xl9535");
    is_found = true;
    // this->pinMode8(0, 0xFF, OUTPUT);
    if (_pwd != GFX_NOT_DEFINED)
    {
      this->pinMode(_pwd, OUTPUT);
      this->digitalWrite(_pwd, 1);
    }
    this->pinMode(_cs, OUTPUT);
    this->digitalWrite(_cs, 1);
    this->pinMode(_sck, OUTPUT);
    this->digitalWrite(_sck, 1);
    this->pinMode(_mosi, OUTPUT);
    this->digitalWrite(_mosi, 1);
  }
  else
  {
    LOG_INFO(TAG_DRVR, "xl9535 not found");
    is_found = false;
  }

  return is_found;
}

void Arduino_PCA9535SWSPI::beginWrite()
{
  this->digitalWrite(_cs, 0);
}

void Arduino_PCA9535SWSPI::endWrite()
{
  this->digitalWrite(_cs, 1);
}

void Arduino_PCA9535SWSPI::writeCommand(uint8_t c)
{
  // D/C bit, command
  this->digitalWrite(_mosi, 0);
  this->digitalWrite(_sck, 0);
  this->digitalWrite(_sck, 1);

  uint8_t bit = 0x80;
  while (bit)
  {
    if (c & bit)
    {
      this->digitalWrite(_mosi, 1);
    }
    else
    {
      this->digitalWrite(_mosi, 0);
    }
    this->digitalWrite(_sck, 0);
    bit >>= 1;
    this->digitalWrite(_sck, 1);
  }
}

void Arduino_PCA9535SWSPI::writeCommand16(uint16_t)
{
}

void Arduino_PCA9535SWSPI::write(uint8_t d)
{
  // D/C bit, data
  this->digitalWrite(_mosi, 1);
  this->digitalWrite(_sck, 0);
  this->digitalWrite(_sck, 1);

  uint8_t bit = 0x80;
  while (bit)
  {
    if (d & bit)
    {
      this->digitalWrite(_mosi, 1);
    }
    else
    {
      this->digitalWrite(_mosi, 0);
    }
    this->digitalWrite(_sck, 0);
    bit >>= 1;
    this->digitalWrite(_sck, 1);
  }
}

void Arduino_PCA9535SWSPI::write16(uint16_t)
{
  // not implemented
}

void Arduino_PCA9535SWSPI::writeRepeat(uint16_t p, uint32_t len)
{
  // not implemented
}

void Arduino_PCA9535SWSPI::writePixels(uint16_t *data, uint32_t len)
{
  // not implemented
}

#if !defined(LITTLE_FOOT_PRINT)
void Arduino_PCA9535SWSPI::writeBytes(uint8_t *data, uint32_t len)
{
  // not implemented
}
#endif // !defined(LITTLE_FOOT_PRINT)

void Arduino_PCA9535SWSPI::writeRegister(uint8_t reg, uint8_t *data, size_t len)
{
  _wire->beginTransmission(_address);
  _wire->write(reg);
  for (size_t i = 0; i < len; i++)
  {
    _wire->write(data[i]);
  }
  _wire->endTransmission();
}

uint8_t Arduino_PCA9535SWSPI::readRegister(uint8_t reg, uint8_t *data, size_t len)
{
  _wire->beginTransmission(_address);
  _wire->write(reg);
  _wire->endTransmission();
  _wire->requestFrom(_address, len);
  size_t index = 0;
  while (index < len)
    data[index++] = _wire->read();
  return 0;
}

void Arduino_PCA9535SWSPI::pinMode(uint8_t pin, uint8_t mode)
{
  if (is_found)
  {
    uint8_t port = 0;
    if (pin > 15)
    {  
      gpio_set_direction((gpio_num_t)pin, mode == OUTPUT ? GPIO_MODE_OUTPUT : GPIO_MODE_INPUT);
    }
    else if (pin > 7)
    {
      this->readRegister(PCA9535_CONFIG_PORT_1_REG, &port, 1);
      if (mode == OUTPUT)
      {
        port = port & (~(1 << (pin - 10)));
      }
      else
      {
        port = port | (1 << (pin - 10));
      }
      this->writeRegister(PCA9535_CONFIG_PORT_1_REG, &port, 1);
    }
    else
    {
      this->readRegister(PCA9535_CONFIG_PORT_0_REG, &port, 1);
      if (mode == OUTPUT)
      {
        port = port & (~(1 << pin));
      }
      else
      {
        port = port | (1 << pin);
      }
      this->writeRegister(PCA9535_CONFIG_PORT_0_REG, &port, 1);
    }
  }
  else
  {
    LOG_INFO(TAG_DRVR, "xl9535 not found");
  }
}
void Arduino_PCA9535SWSPI::pinMode8(uint8_t port, uint8_t pin, uint8_t mode)
{
  if (is_found)
  {
    uint8_t _pin = (mode != OUTPUT) ? pin : ~pin;
    if (port)
    {
      this->writeRegister(PCA9535_CONFIG_PORT_1_REG, &_pin, 1);
    }
    else
    {
      this->writeRegister(PCA9535_CONFIG_PORT_0_REG, &_pin, 1);
    }
  }
  else
  {
    LOG_INFO(TAG_DRVR, "xl9535 not found");
  }
}

void Arduino_PCA9535SWSPI::digitalWrite(uint8_t pin, uint8_t val)
{
  if (is_found)
  {
    uint8_t port = 0;
    uint8_t reg_data = 0;
    if (pin > 15)
    {  
      gpio_set_level((gpio_num_t)pin, val);
    }
    else if (pin > 7)
    {
      this->readRegister(PCA9535_OUTPUT_PORT_1_REG, &reg_data, 1);
      reg_data = reg_data & (~(1 << (pin - 10)));
      port = reg_data | val << (pin - 10);
      this->writeRegister(PCA9535_OUTPUT_PORT_1_REG, &port, 1);
    }
    else
    {
      this->readRegister(PCA9535_OUTPUT_PORT_0_REG, &reg_data, 1);
      reg_data = reg_data & (~(1 << pin));
      port = reg_data | val << pin;
      this->writeRegister(PCA9535_OUTPUT_PORT_0_REG, &port, 1);
    }
  }
  else
  {
    LOG_INFO(TAG_DRVR, "xl9535 not found");
  }
}

int Arduino_PCA9535SWSPI::digitalRead(uint8_t pin)
{
  if (is_found)
  {
    int state = 0;
    uint8_t port = 0;
    if (pin > 15)
    {  
      state = gpio_get_level((gpio_num_t)pin);
    }
    else if (pin > 7)
    {
      this->readRegister(PCA9535_INPUT_PORT_1_REG, &port, 1);
      state = port & (pin - 10) ? 1 : 0;
    }
    else
    {
      this->readRegister(PCA9535_INPUT_PORT_0_REG, &port, 1);
      state = port & pin ? 1 : 0;
    }
    return state;
  }
  else
  {
    LOG_INFO(TAG_DRVR, "xl9535 not found");
  }
  return 0;
}

#endif
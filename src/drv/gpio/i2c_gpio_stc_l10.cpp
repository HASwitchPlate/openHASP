/* MIT License - Copyright (c) 2019-2024 Francis Van Roie */

#include "hasp_conf.h"

#if defined(HASP_USE_I2C_GPIO) && defined(ARDUINO) && defined(HASP_EXPANDER_GPIO_STC_L10)

#include "i2c_gpio_stc_l10.h"

namespace {

/** Max WPR read length across k_pin_map (e.g. motion @ 0x1B). */
constexpr size_t k_read_buf_max = 32u;

uint8_t read_nbytes(const i2c_gpio_pin_map_t* m)
{
    if(m->read_pl_byte > 0) return m->read_pl_byte;
    if(m->isWPR && m->pre_read_len > 0) return m->pre_read_len;
    return 1;
}

/** read_pin_byte is 1-based in buffer; 0 or 0xFF selects last byte. */
uint8_t data_byte_index(const i2c_gpio_pin_map_t* m, uint8_t nbytes)
{
    if(nbytes == 0) return 0;
    if(m->read_pin_byte == 0 || m->read_pin_byte == 0xFFu)
        return (uint8_t)(nbytes - 1u);
    if(m->read_pin_byte >= 1u && m->read_pin_byte <= nbytes)
        return (uint8_t)(m->read_pin_byte - 1u);
    return (uint8_t)(nbytes - 1u);
}

void append_post_write(TwoWire* wire, const i2c_gpio_pin_map_t* m)
{
    if(!m || m->post_write_len == 0) return;
    for(uint8_t i = 0; i < m->post_write_len; i++) {
        wire->write(m->post_write[i]);
    }
}

/* Relays 129–132: keep init_reg on STC path (0x21); 0x01 is TCA9554 output on this bridge and can blank the panel. */
const i2c_gpio_pin_map_t k_pin_map[] = {
    {.pin = 129u, .read_reg = 0x24, .isWPR = true, .read_pin_byte = 4u, .pre_read_len = 4u, .write_reg = 0x25, .init_reg = 0x21, .bit = 0u, .pre_write = {}, .pre_write_len = 0u, .write_seq = {0x01, 0x00, 0x00, 0x00}, .write_seq_len = 4u, .is_digital = false},
    {.pin = 130u, .read_reg = 0x24, .isWPR = true, .read_pin_byte = 4u, .pre_read_len = 4u, .write_reg = 0x25, .init_reg = 0x21, .bit = 1u, .pre_write = {}, .pre_write_len = 0u, .write_seq = {0x01, 0x00, 0x00, 0x00}, .write_seq_len = 4u, .is_digital = false},
    {.pin = 131u, .read_reg = 0x24, .isWPR = true, .read_pin_byte = 4u, .pre_read_len = 4u, .write_reg = 0x25, .init_reg = 0x21, .bit = 2u, .pre_write = {}, .pre_write_len = 0u, .write_seq = {0x01, 0x00, 0x00, 0x00}, .write_seq_len = 4u, .is_digital = false},
    {.pin = 132u, .read_reg = 0x24, .isWPR = true, .read_pin_byte = 4u, .pre_read_len = 4u, .write_reg = 0x25, .init_reg = 0x21, .bit = 3u, .pre_write = {}, .pre_write_len = 0u, .write_seq = {0x01, 0x00, 0x00, 0x00}, .write_seq_len = 4u, .is_digital = false},
    /* Display brightness */
    {.pin = 133u, .read_reg = 0x21, .isWPR = true, .read_pin_byte = 2u, .pre_read_len = 5u, .write_reg = 0x21, .init_reg = 0x21, .bit = 3u, .pre_write_len = 0u, .write_seq = {0x01}, .write_seq_len = 1u, .post_write = {0x00, 0x00, 0x14}, .post_write_len = 3u, .is_digital = false},
    /* Motion: write pointer 0x1B, 20-byte read; byte 18 == 0x07 => present, 0x00 => not (I2C capture). */
    {.pin = 134u, .read_reg = 0x1B, .isWPR = true, .read_pin_byte = 16u, .pre_read_len = 20u, .write_reg = 0x1B, .init_reg = 0x1B, .bit = 0u, .pre_write_len = 0u, .write_seq_len = 0u, .is_digital = true, .on_value = 0x01, .off_value = 0x00},
};
const size_t k_pin_map_count = sizeof(k_pin_map) / sizeof(k_pin_map[0]);

} // namespace

const i2c_gpio_pin_map_t* i2c_gpio_stc_l10::find_map(uint8_t pin) const
{
    for(size_t i = 0; i < k_pin_map_count; i++) {
        if(k_pin_map[i].pin == pin) return &k_pin_map[i];
    }
    return nullptr;
}

bool i2c_gpio_stc_l10::handles_pin(uint8_t pin) const
{
    return find_map(pin) != nullptr;
}

i2c_gpio_stc_l10::i2c_gpio_stc_l10(uint8_t i2c_addr, TwoWire& wire)
{
    _addr = i2c_addr;
    _wire = &wire;
}

bool i2c_gpio_stc_l10::begin()
{
    if(k_pin_map_count == 0) return false;
    uint8_t v;
    return read_reg(k_pin_map[0].read_reg, v);
}

bool i2c_gpio_stc_l10::pre_write(const i2c_gpio_pin_map_t* m)
{
    if(!m || m->pre_write_len == 0) return true;

    _wire->beginTransmission(_addr);
    for(uint8_t i = 0; i < m->pre_write_len; i++) {
        _wire->write(m->pre_write[i]);
    }
    append_post_write(_wire, m);
    return _wire->endTransmission() == 0;
}

bool i2c_gpio_stc_l10::read_reg(uint8_t reg, uint8_t& val)
{
    _wire->beginTransmission(_addr);
    _wire->write(reg);
    if(_wire->endTransmission(false) != 0) return false;
    if(_wire->requestFrom((int)_addr, 1) != 1) return false;
    val = _wire->read();
    return true;
}

bool i2c_gpio_stc_l10::writepointer_read(uint8_t reg, uint8_t* val, uint8_t pre_read_len)
{
    _wire->beginTransmission(_addr);
    _wire->write(reg);
    if(_wire->endTransmission(false) != 0) return false;
    if(_wire->requestFrom((int)_addr, (int)pre_read_len) != (int)pre_read_len) return false;
    for(uint8_t i = 0; i < pre_read_len; i++) {
        if(!_wire->available()) return false;
        val[i] = _wire->read();
    }
    return true;
}

bool i2c_gpio_stc_l10::write_reg(const i2c_gpio_pin_map_t* m, uint8_t reg, uint8_t val)
{
    _wire->beginTransmission(_addr);
    _wire->write(reg);
    if(m && m->write_seq_len > 0) {
        for(uint8_t i = 0; i < m->write_seq_len; i++) {
            _wire->write(m->write_seq[i]);
        }
    }
    _wire->write(val);
    append_post_write(_wire, m);
    return _wire->endTransmission() == 0;
}

bool i2c_gpio_stc_l10::pinMode(uint8_t pin, uint8_t mode)
{
    const i2c_gpio_pin_map_t* m = find_map(pin);
    if(!m) return false;

    uint8_t v;
    if(!read_reg(m->read_reg, v)) return false;

    uint8_t mask = (uint8_t)(1U << m->bit);
    if(mode == OUTPUT)
        v &= (uint8_t)~mask;
    else
        v |= mask;
    if(!pre_write(m)) return false;
    return write_reg(m, m->read_reg, v);
}

bool i2c_gpio_stc_l10::digitalWrite(uint8_t pin, uint8_t val)
{
    const i2c_gpio_pin_map_t* m = find_map(pin);
    if(!m) return false;

    uint8_t rd[k_read_buf_max] = {0};
    const uint8_t nbytes = read_nbytes(m);
    if(nbytes == 0 || nbytes > sizeof(rd)) return false;

    _wire->beginTransmission(_addr);
    _wire->write(m->read_reg);
    if(_wire->endTransmission(false) != 0) return false;

    uint8_t got = _wire->requestFrom((int)_addr, (int)nbytes);
    if(got != nbytes) return false;

    for(uint8_t i = 0; i < nbytes; i++) {
        if(!_wire->available()) return false;
        rd[i] = _wire->read();
    }

    uint8_t relay = rd[data_byte_index(m, nbytes)];
    uint8_t mask  = (uint8_t)(1U << m->bit);

    if(val)
        relay |= mask;
    else
        relay &= (uint8_t)~mask;

    if(m->pre_write_len > 0) {
        _wire->beginTransmission(_addr);
        _wire->write(m->init_reg);
        for(uint8_t i = 0; i < m->pre_write_len; i++) {
            _wire->write(m->pre_write[i]);
        }
        if(_wire->endTransmission() != 0) return false;
    }

    if(m->write_seq_len > sizeof(m->write_seq) || m->post_write_len > sizeof(m->post_write)) return false;

    _wire->beginTransmission(_addr);
    _wire->write(m->write_reg);
    for(uint8_t i = 0; i < m->write_seq_len; i++) {
        _wire->write(m->write_seq[i]);
    }
    _wire->write(relay);
    append_post_write(_wire, m);
    if(_wire->endTransmission() != 0) return false;

    return true;
}

int i2c_gpio_stc_l10::digitalRead(uint8_t pin)
{
    const i2c_gpio_pin_map_t* m = find_map(pin);
    if(!m) return -1;
    if(m->is_digital) return analogReadAndCompare(m, pin);
    return digitalRead_internal(m);
}

int i2c_gpio_stc_l10::digitalRead_internal(const i2c_gpio_pin_map_t* m)
{
    const uint8_t nbytes = read_nbytes(m);
    if(nbytes <= 1) {
        uint8_t v;
        if(!read_reg(m->read_reg, v)) return -1;
        return (int)((v >> m->bit) & 1u);
    }

    uint8_t buf[k_read_buf_max];
    if(nbytes > sizeof(buf)) return -1;
    if(!writepointer_read(m->read_reg, buf, nbytes)) return -1;

    uint8_t b = buf[data_byte_index(m, nbytes)];
    return (int)((b >> m->bit) & 1u);
}

int i2c_gpio_stc_l10::analogReadAndCompare(const i2c_gpio_pin_map_t* m, uint8_t pin)
{
    const int raw = analogRead8(pin);
    if(raw < 0) return -1;
    const uint8_t b = (uint8_t)raw;
    if(b == m->on_value) return 1;
    if(b == m->off_value) return 0;
    return -1;
}

bool i2c_gpio_stc_l10::digitalReadBytes(uint8_t pin, uint8_t* val, uint8_t read_len)
{
    const i2c_gpio_pin_map_t* m = find_map(pin);
    if(!m) return false;
    return writepointer_read(m->read_reg, val, read_len);
}

bool i2c_gpio_stc_l10::analogWrite8(uint8_t pin, uint8_t value)
{
    const i2c_gpio_pin_map_t* m = find_map(pin);
    if(!m) return false;
    if(!pre_write(m)) return false;
    return write_reg(m, m->write_reg, value);
}

int i2c_gpio_stc_l10::analogRead8(uint8_t pin)
{
    const i2c_gpio_pin_map_t* m = find_map(pin);
    if(!m) return -1;

    const uint8_t nbytes = read_nbytes(m);
    if(nbytes <= 1) {
        uint8_t v;
        if(!read_reg(m->read_reg, v)) return -1;
        return (int)v;
    }

    uint8_t buf[k_read_buf_max];
    if(nbytes > sizeof(buf)) return -1;
    if(!writepointer_read(m->read_reg, buf, nbytes)) return -1;
    const uint8_t idx = data_byte_index(m, nbytes);
    return (int)buf[idx];
}

#endif

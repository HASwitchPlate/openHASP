/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_DEVICE_ESP32_H
#define HASP_DEVICE_ESP32_H

#include "hasp_conf.h"
#include "../device.h"

#if defined(ESP32)
#include "driver/ledc.h"

#ifndef BACKLIGHT_FREQUENCY
#define BACKLIGHT_FREQUENCY 20000
#endif

#define BACKLIGHT_FADEMS 200

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(CONFIG_IDF_TARGET_ESP32S2)
uint8_t temprature_sens_read();
#endif

#ifdef __cplusplus
}
#endif

namespace dev {

class Esp32Device : public BaseDevice {

  public:
    Esp32Device();

    void reboot() override;
    void show_info() override;

    const char* get_core_version();
    const char* get_chip_model();
    const char* get_hardware_id();

    void set_backlight_pin(uint8_t pin) override;
    void set_backlight_invert(bool invert) override;
    void set_backlight_level(uint8_t val) override;
    uint8_t get_backlight_level() override;
    void set_backlight_power(bool power) override;
    bool get_backlight_invert() override;
    bool get_backlight_power() override;

    size_t get_free_max_block() override;
    size_t get_free_heap() override;
    uint8_t get_heap_fragmentation() override;
    uint16_t get_cpu_frequency() override;
    void get_info(JsonDocument& doc) override;
    void get_sensors(JsonDocument& doc) override;
    long get_uptime();

    bool is_system_pin(uint8_t pin) override;

  private:
    std::string _hardware_id;
    std::string _chip_model;
    uint32_t _sketch_size; // cached because function is slow

    uint8_t _backlight_pin;
    uint8_t _backlight_level;
    uint8_t _backlight_power;
    uint8_t _backlight_invert;
    bool _backlight_pending;
    bool _backlight_fading;
    bool _backlight_fade;

    void update_backlight();
    static bool cb_backlight(const ledc_cb_param_t *param, void *user_arg);
    void end_backlight_fade();
};

} // namespace dev

#if defined(LANBONL8)
// #warning Building for Lanbon L8
#include "lanbonl8.h"
#elif defined(M5STACK) || defined (M5STACKLGFX)
// #warning Building for M5Stack core2
#include "m5stackcore2.h"
#else
using dev::Esp32Device;
extern dev::Esp32Device haspDevice;
#endif
#endif // ESP32

#endif // HASP_DEVICE_ESP32_H
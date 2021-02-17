#if defined(ESP32)

#include "Arduino.h"
#include <Esp.h>
#include "esp_system.h"

#include "../device.h"
#include "esp32.h"

#include "driver/adc.h"
#include "esp_adc_cal.h"

#include "hasp_conf.h"
#include "hasp_debug.h"

#define BACKLIGHT_CHANNEL 0

namespace dev {

void Esp32Device::reboot()
{
    ESP.restart();
}

void Esp32Device::set_backlight_pin(uint8_t pin)
{
    Esp32Device::backlight_pin = pin;
    /* Setup Backlight Control Pin */
    if(pin >= 0) {
        LOG_VERBOSE(TAG_GUI, F("Backlight  : Pin %d"), pin);

        ledcSetup(BACKLIGHT_CHANNEL, 20000, 12);
        ledcAttachPin(pin, BACKLIGHT_CHANNEL);

        update_backlight();
    }
}

void Esp32Device::set_backlight_level(uint8_t level)
{
    backlight_level = level >= 0 ? level : 0;
    backlight_level = backlight_level <= 100 ? backlight_level : 100;

    update_backlight();
}

uint8_t Esp32Device::get_backlight_level()
{
    return backlight_level;
}

void Esp32Device::set_backlight_power(bool power)
{
    backlight_power = power;
    update_backlight();
}

bool Esp32Device::get_backlight_power()
{
    return backlight_power != 0;
}

void Esp32Device::update_backlight()
{
    if(backlight_pin == -1) return;

    if(backlight_power) {                                                    // The backlight is ON
        ledcWrite(BACKLIGHT_CHANNEL, map(backlight_level, 0, 100, 0, 4095)); // ledChannel and value
    }
}

size_t Esp32Device::get_free_max_block()
{
    return ESP.getMaxAllocHeap();
}

size_t Esp32Device::get_free_heap()
{
    return ESP.getFreeHeap();
}

uint8_t Esp32Device::get_heap_fragmentation()
{
    uint32_t free = ESP.getFreeHeap();
    if(free) {
        return (int8_t)(100.00f - (float)ESP.getMaxAllocHeap() * 100.00f / (float)free);
    } else {
        return 100; // no free memory
    }
}

uint16_t Esp32Device::get_cpu_frequency()
{
    return ESP.getCpuFreqMHz();
}

} // namespace dev

#if defined(LANBONL8)
#warning Building for Lanbon L8
#include "dev/esp32/lanbonl8.h"
#elif defined(M5STACK)
#warning Building for M5Stack core2
#include "dev/esp32/m5stackcore2.h"
#else
dev::Esp32Device haspDevice;
#endif

#endif // ESP32
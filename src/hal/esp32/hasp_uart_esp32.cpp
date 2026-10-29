// MIT License. ESP32 UART implementation; no command/event encoding here.
#include "hasplib.h"
#if defined(HASP_USE_UART_TRANSPORT) && HASP_USE_UART_TRANSPORT > 0 && defined(ESP32) && !defined(HASP_UART_EXTERNAL_HAL)
#if HASP_USE_LVGL_TASK && !defined(HASP_USE_ESP_MQTT)
#error "A separate LVGL task requires openHASP's GUI mutex support"
#endif
#include "hal/hasp_uart.h"
#include "dev/device.h"
#include "drv/tft/tft_driver.h"
#include "sys/gpio/hasp_gpio.h"
#include "hasp_debug.h"
#include "HardwareSerial.h"
#include "driver/gpio.h"
#include "soc/soc_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace hasp_uart { namespace hal {
namespace {
HardwareSerial* uart = nullptr;
SemaphoreHandle_t txMutex = nullptr;
int rxPin = -1, txPin = -1;
bool pinAvailable(int pin, bool output) {
    if(pin < 0 || pin >= SOC_GPIO_PIN_COUNT || !GPIO_IS_VALID_GPIO(pin)) return false;
    if(output && !GPIO_IS_VALID_OUTPUT_GPIO(pin)) return false;
    if(haspDevice.is_system_pin(pin) || haspTft.is_driver_pin(pin)) return false;
#if HASP_USE_GPIO > 0
    if(gpioInUse(pin)) return false;
#endif
#ifdef TFT_BCKL
    if(pin == TFT_BCKL) return false;
#endif
#ifdef TOUCH_SDA
    if(pin == TOUCH_SDA) return false;
#endif
#ifdef TOUCH_SCL
    if(pin == TOUCH_SCL) return false;
#endif
#ifdef TOUCH_IRQ
    if(pin == TOUCH_IRQ) return false;
#endif
#ifdef TOUCH_RST
    if(pin == TOUCH_RST) return false;
#endif
    // Reserve default console pins even if logging is currently disabled.
    if(pin == RX || pin == TX) return false;
#if ARDUINO_USB_CDC_ON_BOOT && defined(CONFIG_IDF_TARGET_ESP32S3)
    if(pin == 19 || pin == 20) return false;
#endif
    return true;
}
}
bool begin(const Config& config) {
    int port = config.port, rx = config.rx, tx = config.tx;
    uint32_t baud = config.baud;
    if(port < 1 || port >= SOC_UART_NUM || rx == tx || baud < 1200 || baud > 921600 ||
       !pinAvailable(rx, false) || !pinAvailable(tx, true)) {
        LOG_ERROR(TAG_CONF, F("Unavailable UART/pins or invalid baud; transport disabled"));
        return false;
    }
#if HASP_USE_GPIO > 0
    for(uint8_t i = 0; i < HASP_NUM_GPIO_CONFIG; ++i) {
        auto gpio = gpioGetPinConfig(i);
        if(port == 1 && (gpio.type == SERIAL_DIMMER || gpio.type == SERIAL_DIMMER_L8_HD ||
                         gpio.type == SERIAL_DIMMER_L8_HD_INVERTED)) {
            LOG_ERROR(TAG_CONF, F("UART1 is reserved by a serial dimmer"));
            return false;
        }
    }
#endif
#if defined(HASP_USE_TASMOTA_CLIENT) && HASP_USE_TASMOTA_CLIENT > 0
    if(port == 2) return false; // Tasmota client owns Serial2.
#endif
    HardwareSerial* selected = &Serial1;
#if SOC_UART_NUM > 2
    if(port == 2) selected = &Serial2;
#endif
    if(static_cast<void*>(selected) == static_cast<void*>(&HASP_SERIAL)) return false;
    txMutex = xSemaphoreCreateMutex();
    if(!txMutex) return false;
    selected->setRxBufferSize(2048);
    selected->begin(baud, SERIAL_8N1, rx, tx);
    if(!*selected) { vSemaphoreDelete(txMutex); txMutex = nullptr; return false; }
    rxPin = rx; txPin = tx;
    uart = selected;
    LOG_INFO(TAG_CONF, F("Generic UART enabled: port %d RX %d TX %d baud %u"), port, rx, tx, baud);
    return true;
}
bool active() { return uart != nullptr; }
int read() { return uart ? uart->read() : -1; }
size_t writeCapacity() {
    int room = uart ? uart->availableForWrite() : 0;
    return room > 0 ? static_cast<size_t>(room) : 0;
}
size_t write(const uint8_t* bytes, size_t size) { return uart ? uart->write(bytes, size) : 0; }
bool pinInUse(uint8_t pin) { return uart && (pin == rxPin || pin == txPin); }
void lock() { if(txMutex) xSemaphoreTake(txMutex, portMAX_DELAY); }
void unlock() { if(txMutex) xSemaphoreGive(txMutex); }
} } // namespace hasp_uart::hal
#endif

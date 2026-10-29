// MIT License. Buildable disabled fallback for platforms without a UART port.
#include "hasplib.h"
#if defined(HASP_USE_UART_TRANSPORT) && HASP_USE_UART_TRANSPORT > 0 && !defined(ESP32) && !defined(HASP_UART_EXTERNAL_HAL)
#include "hasp_uart.h"
#include "hasp_debug.h"
namespace hasp_uart { namespace hal {
bool begin(const Config&) {
    LOG_ERROR(TAG_CONF, F("UART transport has no HAL for this platform"));
    return false;
}
bool active() { return false; }
int read() { return -1; }
size_t writeCapacity() { return 0; }
size_t write(const uint8_t*, size_t) { return 0; }
bool pinInUse(uint8_t) { return false; }
void lock() {}
void unlock() {}
} }
#endif

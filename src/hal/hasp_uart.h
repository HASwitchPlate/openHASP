// MIT License. Portable byte-stream contract for the optional UART transport.
#pragma once
#include <stddef.h>
#include <stdint.h>

namespace hasp_uart { namespace hal {
struct Config {
    int port = 1;
    int rx = -1;
    int tx = -1;
    uint32_t baud = 115200;
};
// Called once at boot. Reject invalid/unavailable ports or pins before enabling.
// Hardware-specific pin numbering and port choices belong in the implementation.
bool begin(const Config& config);
bool active();
// Nonblocking: return byte 0..255, or -1 when no input is immediately available.
int read();
// Write capacity must describe bytes write() can accept without waiting for wire
// transmission. write() returns bytes accepted; short/zero writes are supported.
size_t writeCapacity();
size_t write(const uint8_t* bytes, size_t size);
bool pinInUse(uint8_t pin);
// Serialize all transport state across task callbacks. No ISR calls are made.
// Safe no-ops before begin, when setup is single-threaded. A single-threaded port
// may use no-ops; concurrent ports must supply a real mutex. Never acquire the
// GUI lock here: openHASP calls the loop in its own GUI execution context.
void lock();
void unlock();
} } // namespace hasp_uart::hal

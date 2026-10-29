// MIT License. First-class UART transport; byte I/O is supplied by hal/hasp_uart.h.
#include "hasplib.h"
#if defined(HASP_USE_UART_TRANSPORT) && HASP_USE_UART_TRANSPORT > 0
#include "hasp_uart_transport.h"
#include "hasp_uart_messages.h"
#include "hal/hasp_uart.h"
#include "hasp_debug.h"
#if HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0
#include "hasp_filesystem.h"
#endif

namespace hasp_uart {
UartTransport uartTransport;

void UartTransport::setup()
{
#if HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0
    File file = HASP_FS.open("/serial.json", "r");
    if(!file) return; // Missing configuration means disabled.
    StaticJsonDocument<512> config;
    auto error = deserializeJson(config, file);
    file.close();
    if(error || !config["enabled"].is<bool>()) {
        LOG_ERROR(TAG_CONF, F("Invalid serial.json; UART transport disabled"));
        return;
    }
    if(!config["enabled"].as<bool>()) return;
    if(!config["uart"].is<int>() || !config["rx"].is<int>() || !config["tx"].is<int>() ||
       !config["baud"].is<uint32_t>()) {
        LOG_ERROR(TAG_CONF, F("serial.json requires uart, rx, tx and baud integers"));
        return;
    }
    hal::Config settings;
    settings.port = config["uart"];
    settings.rx = config["rx"];
    settings.tx = config["tx"];
    settings.baud = config["baud"];
    hal::begin(settings);
#endif
}

void UartTransport::onLayoutLoaded()
{
    hal::lock(); // Safe before begin: boot-time layout loading is single-threaded.
    layoutLoaded_ = true;
    if(started_) outbox_.ready();
    hal::unlock();
}

void UartTransport::loop()
{
    if(!hal::active()) return;
    hal::lock();
    bool loaded = layoutLoaded_;
    if(loaded && !started_) {
        outbox_.ready();
        started_ = true;
    }
    hal::unlock();
    if(!loaded) return;
    // This service runs in openHASP's main/GUI context. The HAL never dispatches.
    for(unsigned budget = 0; budget < 256; ++budget) {
        int value = hal::read();
        if(value < 0) break;
        if(receiver_.feed(static_cast<uint8_t>(value))) dispatch_text_line(receiver_.line(), TAG_MSGR);
    }
    hal::lock();
    size_t room = hal::writeCapacity();
    size_t n = outbox_.size();
    if(n > room) n = room;
    if(n > 128) n = 128;
    if(n) outbox_.consume(hal::write(reinterpret_cast<const uint8_t*>(outbox_.data()), n));
    hal::unlock();
}

void UartTransport::publishState(const char* subtopic, const char* payload)
{
    if(!hal::active()) return;
    hal::lock();
    if(started_) forwardState(outbox_, subtopic, payload);
    hal::unlock();
}

bool UartTransport::ownsPin(uint8_t pin) const
{
    return hal::pinInUse(pin);
}

void UartTransport::appendSensorData(JsonDocument& doc)
{
    if(!hal::active()) return;
    hal::lock();
    uint32_t dropped = outbox_.dropped;
    hal::unlock();
    doc["serial"]["dropped"] = dropped;
}

bool UartTransport::handleCustomCommand(const char* topic, const char* payload, uint8_t source)
{
    (void)source;
    if(strcmp(topic, "serialready") || strcmp(payload, "1")) return false;
    hal::lock();
    if(layoutLoaded_ && started_) outbox_.ready();
    hal::unlock();
    return true;
}
} // namespace hasp_uart
#endif

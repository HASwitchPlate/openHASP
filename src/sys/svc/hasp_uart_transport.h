// MIT License. First-class optional UART command and event transport.
#pragma once
#include <stdint.h>
#include <ArduinoJson.h>
#include "hasp_uart_framing.h"

namespace hasp_uart {
class UartTransport {
public:
    void setup();
    void loop();
    void onLayoutLoaded();
    void publishState(const char* subtopic, const char* payload);
    bool ownsPin(uint8_t pin) const;
    void appendSensorData(JsonDocument& doc);
    bool handleCustomCommand(const char* topic, const char* payload, uint8_t source);

private:
    Receiver receiver_;
    Outbox outbox_;
    bool layoutLoaded_ = false;
    bool started_ = false;
};

extern UartTransport uartTransport;
} // namespace hasp_uart

#include <Arduino.h>
#include "ArduinoJson.h"

void gpioSetup()
{
#if defined(ARDUINO_ARCH_ESP8266)
    pinMode(D1, OUTPUT);
    pinMode(D2, INPUT_PULLUP);
#endif
#if defined(STM32_CORE_VERSION)
    pinMode(HASP_OUTPUT_PIN, OUTPUT);
    pinMode(HASP_INPUT_PIN, INPUT_PULLDOWN);
#endif
}
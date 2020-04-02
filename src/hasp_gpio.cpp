#include <Arduino.h>
#include "ArduinoJson.h"

void gpioSetup(JsonObject settings)
{
#if defined(ARDUINO_ARCH_ESP8266)
    pinMode(D1, OUTPUT);
    pinMode(D2, INPUT_PULLUP);
#endif
}
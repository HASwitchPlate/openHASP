#include <Arduino.h>
#include "EEPROM.h"

void eepromSetup()
{

#if defined(STM32Fxx)
    eeprom_buffer_fill();
#endif

    // ESP8266 // Don't start at boot, only at write
    // EEPROM.begin(1024);
    // debugPrintln("EEPROM: Started Eeprom");
}

void eepromLoop()
{}
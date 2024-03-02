/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "hasp_conf.h" // load first

#if HASP_USE_EEPROM > 0

#ifdef ARDUINO
#include <Arduino.h>
#endif

#include "EEPROM.h"

void eepromSetup()
{

#if defined(STM32Fxx)
    eeprom_buffer_fill();
    char buffer[] = "{\"objid\":10,\"id\":1,\"page\":0,\"x\":10,\"y\":45,\"w\":220,\"h\":55,\"toggle\":\"TRUE\","
                    "\"txt\":\"Toggle Me\"}";
    uint size = strlen(buffer);
    uint16_t i;
    for(i = 0; i < size; i++) eeprom_buffered_write_byte(i + 4096, buffer[i]);
    eeprom_buffered_write_byte(i + 4096, 0);
    // eeprom_buffer_flush();
#endif

    // ESP8266 // Don't start at boot, only at write
    // EEPROM.begin(1024);
    // debugPrintln("EEPROM: Started Eeprom");
}

void eepromLoop()
{}

#endif
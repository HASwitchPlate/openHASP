/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_EEPROM_H
#define HASP_EEPROM_H

#include <Arduino.h>

void eepromSetup(void);
void eepromLoop(void);
void eepromWrite(uint16_t addr, std::string& data);
std::string eepromRead(uint16_t addr);

#endif
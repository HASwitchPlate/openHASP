#ifndef HASP_EEPROM_H
#define HASP_EEPROM_H

#include <Arduino.h>

void eepromSetup(void);
void eepromLoop(void);
void eepromWrite(uint16_t addr, std::string & data);
std::string eepromRead(uint16_t addr);

#endif
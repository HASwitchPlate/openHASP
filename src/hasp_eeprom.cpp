#include <EEPROM.h>
#include <Arduino.h>

#include "hasp_debug.h"

void eepromWrite(char addr, std::string & data);
std::string eepromRead(char addr);

void eepromSetup()
{
    EEPROM.begin(1024);
    // debugPrintln("EEPROM: Started Eeprom");
}

void eepromLoop()
{}

void eepromUpdate(uint16_t addr, char ch)
{
    if(EEPROM.read(addr) != ch) {
        EEPROM.write(addr, ch);
    }
}

void eepromWrite(uint16_t addr, std::string & data)
{
    int count = data.length();
    for(int i = 0; i < count; i++) {
        eepromUpdate(addr + i, data[i]);
    }
    eepromUpdate(addr + count, '\0');
    EEPROM.commit();
}

std::string eepromRead(uint16_t addr)
{
    int i;
    char data[1024]; // Max 1024 Bytes
    int len = 0;
    unsigned char k;
    k = EEPROM.read(addr);
    while(k != '\0' && len < 1023) // Read until null character
    {
        k = EEPROM.read(addr + len);
        if((uint8_t(k) < 32) || (uint8_t(k) > 127)) break; // check for printable ascii, includes '\0'
        data[len] = k;
        len++;
    }
    return std::string(data);
}
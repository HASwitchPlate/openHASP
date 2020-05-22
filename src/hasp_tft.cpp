#include "ArduinoJson.h"
#include "ArduinoLog.h"

#ifndef USE_FSMC

#include "TFT_eSPI.h"

#include "hasp_tft.h"
#include "hasp_hal.h"

#if defined(ARDUINO_ARCH_ESP8266)
ADC_MODE(ADC_VCC); // tftShowConfig measures the voltage on the pin
#endif

int8_t getPinName(int8_t pin);

void tftSetup(TFT_eSPI & tft)
{
    tftShowConfig(tft);
}

void tftLoop()
{
    // Nothing to do here
}

void tftStop()
{}

void tftOffsetInfo(uint8_t pin, uint8_t x_offset, uint8_t y_offset)
{
    if(x_offset != 0) {
        Log.verbose(F("TFT: R%u x offset = %i"), pin, x_offset);
    }
    if(y_offset != 0) {
        Log.verbose(F("TFT: R%u y offset = %i"), pin, y_offset);
    }
}

void tftPinInfo(const __FlashStringHelper * pinfunction, int8_t pin)
{
    if(pin != -1) {
        char buffer[64];
        snprintf_P(buffer, sizeof(buffer), PSTR("TFT: %-11s: D%02d (GPIO %02d)"), pinfunction, getPinName(pin), pin);
        Log.verbose(buffer);
    }
}

void tftShowConfig(TFT_eSPI & tft)
{
    setup_t tftSetup;
    tft.getSetup(tftSetup);

    Log.verbose(F("TFT: TFT_eSPI   : v%s"), tftSetup.version.c_str());
    // #if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
    //     Log.verbose(F("TFT: Processor  : ESP%x"), tftSetup.esp);
    // #else
    //     Log.verbose(F("TFT: Processor  : STM%x"), tftSetup.esp);
    // #endif
    Log.verbose(F("TFT: Processor  : %s"), halGetChipModel().c_str());
    Log.verbose(F("TFT: CPU freq.  : %i MHz"), halGetCpuFreqMHz());

#if defined(ARDUINO_ARCH_ESP8266)
    Log.verbose(F("TFT: Voltage    : %2.2f V"), ESP.getVcc() / 918.0); // 918 empirically determined
#endif
    Log.verbose(F("TFT: Transactns : %s"), (tftSetup.trans == 1) ? PSTR("Yes") : PSTR("No"));
    Log.verbose(F("TFT: Interface  : %s"), (tftSetup.serial == 1) ? PSTR("SPI") : PSTR("Parallel"));
#if defined(ARDUINO_ARCH_ESP8266)
    Log.verbose(F("TFT: SPI overlap   : %s"), (tftSetup.overlap == 1) ? PSTR("Yes") : PSTR("No"));
#endif

    if(tftSetup.tft_driver != 0xE9D) // For ePaper displays the size is defined in the sketch
    {
        Log.verbose(F("TFT: Driver     : %s"), halDisplayDriverName().c_str()); // tftSetup.tft_driver);
        Log.verbose(F("TFT: Resolution : %ix%i"), tftSetup.tft_width, tftSetup.tft_height);
    } else if(tftSetup.tft_driver == 0xE9D)
        Log.verbose(F("Driver = ePaper"));

    // Offsets, not all used yet
    tftOffsetInfo(0, tftSetup.r0_x_offset, tftSetup.r0_y_offset);
    tftOffsetInfo(1, tftSetup.r1_x_offset, tftSetup.r1_y_offset);
    tftOffsetInfo(2, tftSetup.r2_x_offset, tftSetup.r2_y_offset);
    tftOffsetInfo(3, tftSetup.r3_x_offset, tftSetup.r3_y_offset);
    /* replaced by tftOffsetInfo
    //    if(tftSetup.r1_x_offset != 0) Serial.printf("R1 x offset = %i \n", tftSetup.r1_x_offset);
    //    if(tftSetup.r1_y_offset != 0) Serial.printf("R1 y offset = %i \n", tftSetup.r1_y_offset);
    //    if(tftSetup.r2_x_offset != 0) Serial.printf("R2 x offset = %i \n", tftSetup.r2_x_offset);
    //    if(tftSetup.r2_y_offset != 0) Serial.printf("R2 y offset = %i \n", tftSetup.r2_y_offset);
    //    if(tftSetup.r3_x_offset != 0) Serial.printf("R3 x offset = %i \n", tftSetup.r3_x_offset);
    //    if(tftSetup.r3_y_offset != 0) Serial.printf("R3 y offset = %i \n", tftSetup.r3_y_offset);
    */

    tftPinInfo(F("MOSI"), tftSetup.pin_tft_mosi);
    tftPinInfo(F("MISO"), tftSetup.pin_tft_miso);
    tftPinInfo(F("SCLK"), tftSetup.pin_tft_clk);

#if defined(ARDUINO_ARCH_ESP8266)
    if(tftSetup.overlap == true) {
        Log.verbose(F("Overlap selected, following pins MUST be used:"));

        Log.verbose(F("MOSI     : SD1 (GPIO 8)"));
        Log.verbose(F("MISO     : SD0 (GPIO 7)"));
        Log.verbose(F("SCK      : CLK (GPIO 6)"));
        Log.verbose(F("TFT_CS   : D3  (GPIO 0)"));

        Log.verbose(F("TFT_DC and TFT_RST pins can be tftSetup defined"));
    }
#endif

    tftPinInfo(F("TFT_CS"), tftSetup.pin_tft_cs);
    tftPinInfo(F("TFT_DC"), tftSetup.pin_tft_dc);
    tftPinInfo(F("TFT_RST"), tftSetup.pin_tft_rst);

    tftPinInfo(F("TOUCH_CS"), tftSetup.pin_tch_cs);

    tftPinInfo(F("TFT_WR"), tftSetup.pin_tft_wr);
    tftPinInfo(F("TFT_RD"), tftSetup.pin_tft_rd);

    tftPinInfo(F("TFT_D0"), tftSetup.pin_tft_d0);
    tftPinInfo(F("TFT_D1"), tftSetup.pin_tft_d1);
    tftPinInfo(F("TFT_D2"), tftSetup.pin_tft_d2);
    tftPinInfo(F("TFT_D3"), tftSetup.pin_tft_d3);
    tftPinInfo(F("TFT_D4"), tftSetup.pin_tft_d4);
    tftPinInfo(F("TFT_D5"), tftSetup.pin_tft_d5);
    tftPinInfo(F("TFT_D6"), tftSetup.pin_tft_d6);
    tftPinInfo(F("TFT_D7"), tftSetup.pin_tft_d7);

    if(tftSetup.serial == 1) {
        Log.verbose(F("TFT: Display SPI freq. : %d.%d MHz"), tftSetup.tft_spi_freq / 10, tftSetup.tft_spi_freq % 10);
    }
    if(tftSetup.pin_tch_cs != -1) {
        Log.verbose(F("TFT: Touch SPI freq.   : %d.%d MHz"), tftSetup.tch_spi_freq / 10, tftSetup.tch_spi_freq % 10);
    }
}

// Get pin name for ESP8266
int8_t getPinName(int8_t pin)
{
// For ESP32 and STM32 pin labels on boards use the GPIO number
#ifndef ARDUINO_ARCH_ESP8266
    return pin;
#endif

    // For ESP8266 the pin labels are not the same as the GPIO number
    // These are for the NodeMCU pin definitions:
    //        GPIO       Dxx
    switch(pin) {
        case 16:
            return 0;
        case 5:
            return 1;
        case 4:
            return 2;
        case 0:
            return 3;
        case 2:
            return 4;
        case 14:
            return 5;
        case 12:
            return 6;
        case 13:
            return 7;
        case 15:
            return 8;
        case 3:
            return 9;
        case 1:
            return 10;
        case 9:
            return 11;
        case 10:
            return 12;
        default:
            return -1; // Invalid pin
    }
}

#endif

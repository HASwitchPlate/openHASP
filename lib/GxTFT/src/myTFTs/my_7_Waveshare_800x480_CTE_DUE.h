// e.g. https://www.aliexpress.com/item/7inch-Capacitive-Touch-LCD-C-800-480-Multicolor-Graphic-LCD-TFT-I2C-Touch-Panel-Interface/1968103773.html
// e.g. on https://www.aliexpress.com/item/New-TFT-SD-Shield-for-Arduino-DUE-TFT-LCD-Module-SD-Card-Adapter-2-8-3/32709157722.html

// IMPORTANT Note: DO NOT connect the display directly to the CTE shield!
//                 only connector pins 1..24 can be connected directly; connect external 3.3V to pin 33, with GND to 34
//                 (I use a long tails connector in between, with pins 25..34 bent sidewards)

#include "../GxIO/GxIO_DUE_P16_DUESHIELD/GxIO_DUE_P16_DUESHIELD.h"
#include "../GxCTRL/GxCTRL_RA8875P/GxCTRL_RA8875P.h" // 800x480 e.g. Waveshare 7inch Display
GxIO_Class io; // #define GxIO_Class is in the selected header file
GxCTRL_Class controller(io); // #define GxCTRL_Class is in the selected header file
//TFT_Class tft(io, controller, 480, 800); // portrait 800x480 7inch Display
TFT_Class tft(io, controller, 800, 480); // landscape 800x480 7inch Display

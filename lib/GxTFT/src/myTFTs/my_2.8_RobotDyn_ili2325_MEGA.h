// e.g. https://www.aliexpress.com/item/TFT-2-8-LCD-Touch-Screen-module-3-3V-with-SD-and-MicroSD-card/32711829802.html

// note: the display needs VCC supply on LED-A pin, port output is not enough
//       VCC should be 5V, works also with 3.3V (rather dim if LED-A is 3.3V)
//       data lines 3.3V, use shield with level converters or series resistors

#include "../GxIO/GxIO_MEGA_P8_MEGASHIELD_H/GxIO_MEGA_P8_MEGASHIELD_H.h"
#include "../GxCTRL/GxCTRL_ILI9325C/GxCTRL_ILI9325C.h" // 240x320
GxIO_Class io; // #define GxIO_Class is in the selected header file
GxCTRL_Class controller(io); // #define GxCTRL_Class is in the selected header file
TFT_Class tft(io, controller, 240, 320); // portrait 240x320
//TFT_Class tft(io, controller, 320, 240); // landscape 240x320

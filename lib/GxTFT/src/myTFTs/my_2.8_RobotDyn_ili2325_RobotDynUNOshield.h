// e.g. https://www.aliexpress.com/item/TFT-2-8-LCD-Touch-Screen-module-3-3V-with-SD-and-MicroSD-card/32711829802.html
// on   https://www.aliexpress.com/store/product/Expansion-Shield-for-TFT-2-8-LCD-Touch-Screen-for-Uno-Mega/1950989_32711885041.html

// note: the display needs VCC supply on LED-A pin, port output is not enough
//       VCC should be 5V, works also with 3.3V (rather dim if LED-A is 3.3V)
//       data lines 3.3V, use shield with level converters or series resistors
//       these requirements are met with this shield
//
// note: the shield uses the full PORTD for data, will produce garbage on TX pin

#include "../GxIO/GxIO_UNO_P8_ROBOTDYN_SHIELD/GxIO_UNO_P8_ROBOTDYN_SHIELD.h"
#include "../GxCTRL/GxCTRL_ILI9325C/GxCTRL_ILI9325C.h" // 240x320
GxIO_Class io; // #define GxIO_Class is in the selected header file
GxCTRL_Class controller(io); // #define GxCTRL_Class is in the selected header file
TFT_Class tft(io, controller, 240, 320); // portrait 240x320
//TFT_Class tft(io, controller, 320, 240); // landscape 240x320

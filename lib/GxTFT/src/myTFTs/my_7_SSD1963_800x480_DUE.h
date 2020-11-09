// e.g. https://www.aliexpress.com/item/New-7-inch-TFT-LCD-module-800x480-SSD1963-Touch-PWM-For-Arduino-AVR-STM32-ARM/32667404985.html

#include "../GxIO/GxIO_DUE_P16_DUESHIELD/GxIO_DUE_P16_DUESHIELD.h"
#include "../GxCTRL/GxCTRL_SSD1963/GxCTRL_SSD1963.h" // 800x480 e.g. 7inch Display
GxIO_Class io; // #define GxIO_Class is in the selected header file
GxCTRL_Class controller(io); // #define GxCTRL_Class is in the selected header file
TFT_Class tft(io, controller, 800, 480); // landscape 800x480 7inch Display

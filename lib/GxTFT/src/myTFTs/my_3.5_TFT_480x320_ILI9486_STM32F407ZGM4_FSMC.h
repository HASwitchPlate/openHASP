// e.g. https://www.aliexpress.com/item/Smart-Electronics-3-5-inch-TFT-Touch-Screen-LCD-Module-Display-320-480-ILI9486-with-PCB/32586941686.html
// on   https://www.aliexpress.com/item/STM32F407ZGT6-Development-Board-ARM-M4-STM32F4-cortex-M4-core-Board-Compatibility-Multiple-Extension/32795142050.html
//
// this version is for use with Arduino package STM32GENERIC, board "BLACK F407VE/ZE/ZG boards".
// Specific Board "BLACK F407ZG (M4 DEMO)"

#include "../GxIO/STM32GENERIC/GxIO_STM32F407ZGM4_FSMC/GxIO_STM32F407ZGM4_FSMC.h"
#include "../GxCTRL/GxCTRL_ILI9486/GxCTRL_ILI9486.h" // 320x480
GxIO_Class io; // #define GxIO_Class is in the selected header file
GxCTRL_Class controller(io); // #define GxCTRL_Class is in the selected header file
TFT_Class tft(io, controller, 480, 320); // landscape 320x480
//TFT_Class tft(io, controller, 320, 480); // portrait 320x480

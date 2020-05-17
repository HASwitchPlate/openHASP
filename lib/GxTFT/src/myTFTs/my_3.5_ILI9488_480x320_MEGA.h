// e.g. https://www.aliexpress.com/item/1pcs-3-5-inch-TFT-LCD-screen-module-for-Arduino-Mega-2560-R3-Mega2560-REV3-ATmega2560/1905714128.html
//      the information on the above website is misleading

#include "../GxIO/GxIO_MEGA_P16_MEGASHIELD/GxIO_MEGA_P16_MEGASHIELD.h"
#include "../GxCTRL/GxCTRL_ILI9488/GxCTRL_ILI9488.h" // 320x480 e.g. 3.5inch MEGA
GxIO_Class io; // #define GxIO_Class is in the selected header file
GxCTRL_Class controller(io); // #define GxCTRL_Class is in the selected header file
//TFT_Class tft(io, controller, 320, 480); // portrait HVGA 320x480 or 3.5inch RPI Display
TFT_Class tft(io, controller, 480, 320); // landscape HVGA 320x480 or 3.5inch RPI Display


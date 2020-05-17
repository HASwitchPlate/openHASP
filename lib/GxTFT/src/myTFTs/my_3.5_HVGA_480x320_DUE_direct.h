// e.g. https://www.aliexpress.com/item/3-2-inch-TFT-LCD-screen-module-Ultra-HD-320X480-for-Arduino-MEGA-2560-R3-Board/32604352555.html

#include "../GxIO/GxIO_DUE_P16_HVGASHIELD/GxIO_DUE_P16_HVGASHIELD.h"
#include "../GxCTRL/GxCTRL_ILI9481/GxCTRL_ILI9481.h" // HVGA 320x480
GxIO_Class io; // #define GxIO_Class is in the selected header file
GxCTRL_Class controller(io); // #define GxCTRL_Class is in the selected header file
TFT_Class tft(io, controller, 480, 320); // landscape HVGA 320x480 or 3.5inch RPI Display

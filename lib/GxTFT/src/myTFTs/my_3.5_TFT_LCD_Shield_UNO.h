// e.g. https://www.aliexpress.com/item/Free-shipping-LCD-Display-Module-TFT-3-5-inch-TFT-LCD-screen-for-Arduino-UNO-R3/32579880571.html

#include "../GxIO/GxIO_UNO_P8_SHIELD/GxIO_UNO_P8_SHIELD.h"
#include "../GxCTRL/GxCTRL_ILI9481/GxCTRL_ILI9481.h" // HVGA 320x480
GxIO_Class io; // #define GxIO_Class is in the selected header file
GxCTRL_Class controller(io); // #define GxCTRL_Class is in the selected header file
TFT_Class tft(io, controller, 480, 320); // landscape HVGA 320x480 or 3.5inch RPI Display

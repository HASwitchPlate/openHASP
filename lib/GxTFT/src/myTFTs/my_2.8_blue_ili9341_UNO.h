// e.g https://www.banggood.com/2_8-Inch-TFT-LCD-Shield-Touch-Display-Module-For-Arduino-UNO-p-989697.html

#include "../GxIO/GxIO_UNO_P8_SHIELD/GxIO_UNO_P8_SHIELD.h"
#include "../GxCTRL/GxCTRL_ILI9341/GxCTRL_ILI9341.h" // 240x320
GxIO_Class io; // #define GxIO_Class is in the selected header file
GxCTRL_Class controller(io); // #define GxCTRL_Class is in the selected header file
TFT_Class tft(io, controller, 240, 320); // portrait 240x320
//TFT_Class tft(io, controller, 320, 240); // landscape 240x320

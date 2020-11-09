// e.g https://www.aliexpress.com/item/ForArduino-UNO-2-4-inch-TFT-touch-screen-supporting-For-UNO-R3/1949318944.html

#include "../GxIO/GxIO_UNO_P8_SHIELD/GxIO_UNO_P8_SHIELD.h"
//#include "../GxCTRL/GxCTRL_ILI9325C/GxCTRL_ILI9325C.h" // 240x320
#include "../GxCTRL/GxCTRL_ILI9325D/GxCTRL_ILI9325D.h" // 240x320
GxIO_Class io; // #define GxIO_Class is in the selected header file
GxCTRL_Class controller(io); // #define GxCTRL_Class is in the selected header file
TFT_Class tft(io, controller, 240, 320); // portrait 240x320
//TFT_Class tft(io, controller, 320, 240); // landscape 240x320

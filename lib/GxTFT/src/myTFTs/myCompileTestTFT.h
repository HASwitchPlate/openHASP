#include "../GxIO/GxIO_UNO_P8_SHIELD/GxIO_UNO_P8_SHIELD.h"
//#include "../GxCTRL/GxCTRL_HX8357B/GxCTRL_HX8357B.h"
#include "../GxCTRL/GxCTRL_HX8357C/GxCTRL_HX8357C.h"
GxIO_Class io; // #define GxIO_Class is in the selected header file
GxCTRL_Class controller(io); // #define GxCTRL_Class is in the selected header file
TFT_Class tft(io, controller, 240, 320); // portrait 240x320

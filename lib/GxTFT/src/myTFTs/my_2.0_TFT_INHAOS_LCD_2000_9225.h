// e.g. https://www.aliexpress.com/item/LCD-Development-Kit-9225-with-Mega-2560-2-0-TFT-LCD-Display-TFT-LCD-PCB-Adapter/32587170491.html

// this display uses 8 bit data bus but has a latch for 16 bit panel

#include "../GxIO/GxIO_MEGA_P8_MEGASHIELD_LS/GxIO_MEGA_P8_MEGASHIELD_LS.h"
#include "../GxCTRL/GxCTRL_ILI9225/GxCTRL_ILI9225.h" // 176x220
GxIO_Class io; // #define GxIO_Class is in the selected header file
GxCTRL_Class controller(io); // #define GxCTRL_Class is in the selected header file
TFT_Class tft(io, controller, 176, 220); // portrait 176x220
//TFT_Class tft(io, controller, 220, 1760); // landscape 176x220

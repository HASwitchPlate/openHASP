// e.g https://www.aliexpress.com/item/TIANMA-2-8-inch-37PIN-TFT-LCD-Module-ILI9341-Drive-IC-240-RGB-320-SPI-RGB/32267693877.html

#include "../GxIO/GxIO_SPI_RS/GxIO_SPI_RS.h"
#include "../GxCTRL/GxCTRL_ILI9341/GxCTRL_ILI9341.h" // 240x320
GxIO_Class io(SPI, SS, -1, 0); // on ESP8266, no dc, rst on GPIO0 (D3 on Wemos D1 mini)
GxCTRL_Class controller(io); // #define GxCTRL_Class is in the selected header file
TFT_Class tft(io, controller, 240, 320); // portrait 240x320
//TFT_Class tft(io, controller, 320, 240); // landscape 240x320

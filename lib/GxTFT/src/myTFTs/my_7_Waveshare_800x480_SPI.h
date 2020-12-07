// e.g. https://www.aliexpress.com/item/7inch-Capacitive-Touch-LCD-C-800-480-Multicolor-Graphic-LCD-TFT-I2C-Touch-Panel-Interface/1968103773.html

#include "../GxIO/GxIO_SPI/GxIO_SPI.h"
#include "../GxCTRL/GxCTRL_RA8875S/GxCTRL_RA8875S.h" // 800x480 e.g. Waveshare 7inch Display
GxIO_Class io(SPI, SS, D4, D3); // on Wemos D1 (ESP8266)
GxCTRL_Class controller(io); // #define GxCTRL_Class is in the selected header file
//TFT_Class tft(io, controller, 480, 800); // portrait 800x480 7inch Display
TFT_Class tft(io, controller, 800, 480); // landscape 800x480 7inch Display

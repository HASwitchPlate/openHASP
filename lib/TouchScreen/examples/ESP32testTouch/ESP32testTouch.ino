/* 
Test MCU Friend parallel display and resistive touchscreen by drawing touch points
on screen, use something pointed for more accuracy

Need this modified Touchscreen library and one of:
- TFT_eSPI        much faster for ESP32, must select correct display driver 
- MCUFRIEND_kbv   more display driver support, auto detects display driver
 */

#define TFT_eSPIlib  // comment out to use MCUFRIEND_kbv

#ifdef TFT_eSPIlib
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();
#else
#include <MCUFRIEND_kbv.h> 
MCUFRIEND_kbv tft;
#endif

#include <TouchScreen.h>

// adjust pressure sensitivity - note works 'backwards'
#define MINPRESSURE 200
#define MAXPRESSURE 1000

// some colours to play with
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
 
// Either run TouchScreen_Calibr_native.ino and apply results to the arrays below
// or just use trial and error from drawing on screen
// ESP32 coordinates at default 12 bit resolution have range 0 - 4095
// however the ADC cannot read voltages below 150mv and tops out around 3.15V
// so the actual coordinates will not be at the extremes
// each library and driver may have different coordination and rotation sequence
const int coords[] = {3800, 500, 300, 3800}; // portrait - left, right, top, bottom

const int rotation = 0; //  in rotation order - portrait, landscape, etc

const int XP = 27, XM = 15, YP = 4, YM = 14; // default ESP32 Uno touchscreen pins
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

void setup() {
    Serial.begin(115200);
#ifdef TFT_eSPIlib
    Serial.println("TFT_eSPI library");
    tft.begin();
#else
    Serial.println("MCUFRIEND_kbv library");
    idDisplay();
#endif
    // screen orientation and background
    String orientation;
    switch (rotation) {
      case 0: 
        orientation = "Portrait";
      break;
      case 1: 
        orientation = "Landscape";
      break;
      case 2: 
        orientation = "Portrait Inverted";
      break;
      case 3: 
        orientation = "Landscape Inverted";
      break;
    }
    Serial.println(orientation);
    tft.setRotation(rotation);  
    tft.fillScreen(BLACK);
}

void loop() {
    // display touched point with colored dot
    uint16_t pixel_x, pixel_y;    
    boolean pressed = Touch_getXY(&pixel_x, &pixel_y, true);
}

boolean Touch_getXY(uint16_t *x, uint16_t *y, boolean showTouch) {
    TSPoint p = ts.getPoint();
    pinMode(YP, OUTPUT);      //restore shared pins
    pinMode(XM, OUTPUT);
    digitalWrite(YP, HIGH);   //because TFT control pins
    digitalWrite(XM, HIGH);
    bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
    if (pressed) {
      switch (rotation) {
        case 0: // portrait
          *x = map(p.x, coords[0], coords[1], 0, tft.width()); 
          *y = map(p.y, coords[2], coords[3], 0, tft.height());
        break;
        case 1: // landscape
          *x = map(p.y, coords[1], coords[0], 0, tft.width()); 
          *y = map(p.x, coords[2], coords[3], 0, tft.height());
        break;
        case 2: // portrait inverted
          *x = map(p.x, coords[1], coords[0], 0, tft.width()); 
          *y = map(p.y, coords[3], coords[2], 0, tft.height());
        break;
        case 3: // landscape inverted
          *x = map(p.y, coords[0], coords[1], 0, tft.width()); 
          *y = map(p.x, coords[3], coords[2], 0, tft.height());
        break;
      }      
      if (showTouch) tft.fillCircle(*x, *y, 2, YELLOW);
    }
    return pressed;
}

#ifndef TFT_eSPIlib
void idDisplay() {
    // MCUFRIEND_kbv library only
    uint16_t ID = tft.readID();
    Serial.print("TFT ID = 0x");
    Serial.println(ID, HEX);
    if (ID == 0xD3D3) ID = 0x9486; // write-only shield
    tft.begin(ID);
}
#endif


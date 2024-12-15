/* MIT License - Copyright (c) 2019-2022 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

// USAGE: - Copy this file and rename it to my_custom.cpp
//        - Change false to true on line 9
//280mA - LED ON, 190mA - LED off

#include "hasplib.h"
//#include <lvgl.h>

#if defined(HASP_USE_CUSTOM) && true // <-- set this to true in your code

#include "hasp_debug.h"
#include "custom/my_custom.h"

unsigned long last_blink = 0;
const int voltage_read = 35;
const int blink_speed = 1000; //read every 1 sec

float batteryFraction;
float currentVoltage;

//extern lv_obj_t* battery_bar;  // Declare the battery bar variable
//lv_obj_t* battery_bar;  // Declare the battery bar variable

// Implement heartbeat led
unsigned long lastMillis = 0;

//Voltage read
const int MAX_ANALOG_VAL = 4095;
const float MAX_BATTERY_VOLTAGE = 4.2; // Max LiPoly voltage of a 3.7 battery is 4.2
const float minVoltage = 3.0;  // Minimum voltage (0% charge)
const float maxVoltage = 4.2;  // Maximum voltage (100% charge)

//deep sleep timer
const int sleepTimeSeconds = 1;  // Set the sleep time in seconds

void custom_setup()
{
    // Initialization code here
    analogReadResolution(12);
    last_blink = millis();
    // pinMode(voltage_read, INPUT_PULLUP);

    randomSeed(millis());
}

void custom_loop()
{
    // read voltage every 60 seconds
    if(blink_speed && (millis() - last_blink > blink_speed)) {

        currentVoltage = analogReadMilliVolts(35);
        currentVoltage = currentVoltage * 2 / 1000;
        Serial.println(currentVoltage);
        
        // Calculate the percentage of charge
        batteryFraction = map(constrain(currentVoltage, minVoltage, maxVoltage)*1000, minVoltage*1000, maxVoltage*1000, 0, 100);
        //read illumination
        Serial.println(batteryFraction);
        last_blink = millis();
        
        updateBatteryDisplay(12, 9, batteryFraction);
        updateBatteryDisplay(9, 9, batteryFraction);
        updateBatteryDisplay(0, 6, batteryFraction);
        //updateVoltageDisplay(9,10,currentVoltage);
        String voltageString = String(currentVoltage, 2);     // Converts the float to a String with 2 decimal places
        voltageString += "V";                                 // Concatenates "V" at the end
        updateTextDisplay(9, 8, voltageString.c_str());
        String fractionString = String(batteryFraction, 2);   // Converts the float to a String with 2 decimal places
        fractionString += "%";                                // Concatenates "%" at the end 
        updateTextDisplay(9, 5, fractionString.c_str());  
        updateTextDisplay(12, 11, fractionString.c_str());
    }

}

//search bar element and apply adata float value to it
void updateBatteryDisplay(uint8_t page, uint8_t id, float adata) {
    lv_obj_t* widget = hasp_find_obj_from_page_id(page, id);
    if (!widget) return; // object doesn't exist

    // Calculate the color based on battery percentage
    uint8_t red = constrain(map(adata, 0, 100, 255, 0), 0, 255);
    uint8_t green = constrain(map(adata, 0, 100, 0, 255), 0, 255);
    uint8_t blue = 0; // Assuming no blue component

    // Set the calculated color
    lv_color_t color = LV_COLOR_MAKE(red, green, blue);
    lv_obj_set_style_local_bg_color(widget, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, color);


    lv_bar_set_value(widget, adata, LV_ANIM_OFF);
}

//search label element and apply adata text value to it
void updateTextDisplay(uint8_t page, uint8_t id, const char* text) {
    lv_obj_t* widget = hasp_find_obj_from_page_id(page, id);
    if (!widget) return; // object doesn't exist
    lv_label_set_text(widget, text);
}

void custom_every_second()
{
    Serial.print("#");
}

void custom_every_5seconds()
{
    LOG_VERBOSE(TAG_CUSTOM, "%d seconds have passsed...", 5);


    // Convert the integer to a string
    String vbatFraction = String(batteryFraction);
    String vbatLevel = String(currentVoltage);

    // Create the JSON string
    String jsonString = "{\"vbat_Fraction\":" + vbatFraction + "}";
    String jsonString2 = "{\"vbat_Level\":" + vbatLevel + "}";

    

    // Convert the JSON string to a const char* for your function
    const char* jsonChar = jsonString.c_str();
    const char* jsonChar2 = jsonString2.c_str();

    // Call your function with the JSON string
    dispatch_state_subtopic("vbat_Fraction", jsonChar);
    dispatch_state_subtopic("vbat_Level", jsonChar2);  
    
    //Battery percentage
    String jsonString4 = "Battery"; //topic
    const char* jsonChar4 = jsonString4.c_str();
    dispatch_state_val(jsonChar4, (hasp_event_t) 1, batteryFraction); 

}

bool custom_pin_in_use(uint8_t pin)
{
    /*
    switch(pin) {
        case illum_read:  // Custom LED pin
        case 6:  // Custom Input pin
            return true;
        default:
            return false;
    }
    */
   return false;
}

void custom_get_sensors(JsonDocument& doc)
{
    JsonObject sensor = doc.createNestedObject(F("Battery"));  // Add Key
    sensor[F("Battery")] = batteryFraction;                        // Set Value

}

void custom_topic_payload(const char* topic, const char* payload, uint8_t source){
    // Not used
}

void custom_state_subtopic(const char* subtopic, const char* payload){

}

#endif // HASP_USE_CUSTOM

/* MIT License - Copyright (c) 2019-2022 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

// USAGE: - Copy this file and rename it to my_custom.cpp
//        - Change false to true on line 9
//280mA - LED ON, 190mA - LED off

#include "hasplib.h"
//#include <lvgl.h>
#include "driver/rtc_io.h"
#include <Adafruit_AHTX0.h>

#if defined(HASP_USE_CUSTOM) && true // <-- set this to true in your code

#include "hasp_debug.h"
#include "custom/my_custom.h"

Adafruit_AHTX0 aht;
unsigned long last_blink = 0;
const int voltage_read = 35;
const int blink_speed = 60000; //read every 60 sec

float batteryFraction;
float currentVoltage;
float AhtTemperature;
float AhtHumidity; 

float lastBatVal = 0;

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
// const int sleepTimeSeconds = 60;  // Set the sleep time in seconds

void custom_setup()
{
    aht.begin();            //0x38
    // Initialization code here
    analogReadResolution(12);
    last_blink = millis();

    touchSleepWakeUpEnable(T0, 66);

    // pinMode(2, OUTPUT); //disable onboard voltage converter for neopixel connector
    // digitalWrite(2, LOW);

    gpio_hold_dis(GPIO_NUM_26);
 
    gpio_deep_sleep_hold_dis();

    randomSeed(millis());
}

void custom_loop()
{


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
    // Serial.print("#");
}

void custom_every_5seconds()
{
    // LOG_VERBOSE(TAG_CUSTOM, "%d seconds have passsed...", 5);
    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp);
    AhtTemperature = temp.temperature;
    AhtHumidity = humidity.relative_humidity;

    //Convert the integer to a string
    String TempFraction = String(temp.temperature, 2);
    String HumLevel = String(humidity.relative_humidity,2);

    // Create the JSON string
    String jsonString = "{\"Temperature\":" + TempFraction + "}";
    String jsonString2 = "{\"Humidity\":" + HumLevel + "}";

    

    // Convert the JSON string to a const char* for your function
    const char* jsonChar = jsonString.c_str();
    const char* jsonChar2 = jsonString2.c_str();

    // Call your function with the JSON string
    dispatch_state_subtopic("Temperature", jsonChar);
    dispatch_state_subtopic("Humidity", jsonChar2);  

    String TempStr = String(temp.temperature, 1) + "°C";
    String HumStr = String(int(humidity.relative_humidity)) + "%";

    updateTextDisplay(0, 2, TempStr.c_str());
    updateTextDisplay(0, 6, HumStr.c_str());


    String jsonString5 = "Humidity"; //topic
    const char* jsonChar5 = jsonString5.c_str();
    dispatch_state_val(jsonChar5, (hasp_event_t) 1, AhtHumidity);

}

bool custom_pin_in_use(uint8_t pin)
{
   return false;
}

void custom_get_sensors(JsonDocument& doc)
{

}

void custom_topic_payload(const char* topic, const char* payload, uint8_t source){
    // Not used
}

void custom_state_subtopic(const char* subtopic, const char* payload){

}

#endif // HASP_USE_CUSTOM

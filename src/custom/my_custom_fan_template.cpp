/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

// USAGE: - Copy this file and rename it to my_custom_fan_rotator.cpp, also copy my_custom_template.h and rename it to my_custom.h
//        - Uncomment in your user_config_override.h the line containing #define HASP_USE_CUSTOM 1
//        - Change false to true on line 16
//        - Set the page and id of your object on lines 27 and 28
//        - Optionally set the default rotation state and angle on lines 21 and 22
//        - At run time you can change fanspeed and fanangle by updating the topics hasp/<yourplate>/custom/fanspeed 
//          and hasp/<yourplate>/custom/fanangle with values desired from your home automation system
// NOTE:  This gives best results with small images (up to 64x64 pixels). Large images may draw choppy due to limited 
//        computing capacity of the MCU.

#include "hasplib.h"

#if defined(HASP_USE_CUSTOM) && HASP_USE_CUSTOM > 0 && false // <-- set this to true in your code

#include "hasp_debug.h"

uint16_t fanspeed       = 0;   // rotation off by default (the time between angle turns in ms)
uint16_t fanangle       = 450; // default angle of one turn (0.1 degree precision, so this means 45°)
unsigned long prev_loop = 0;

static void my_rotate_fan()
{
    uint8_t page = 1;   // the page of the object you want to rotate
    uint8_t id   = 150; // the id of the object you want to rotate

    lv_obj_t* fan = hasp_find_obj_from_page_id(page, id);
    if(!fan) return; // object doesn't exist

    uint16_t angle = lv_img_get_angle(fan) + fanangle;
    lv_img_set_angle(fan, angle % 3600);
}

void custom_setup()
{
    // Initialization code here
    prev_loop = millis();
}

void custom_loop()
{
    // Non-blocking code here, this should execute very fast!
    if(fanspeed && (millis() - prev_loop > fanspeed)) {
        my_rotate_fan();
        prev_loop = millis();
    }
}

void custom_every_second()
{
    // Unused
}

void custom_every_5seconds()
{
    // Unused
}

bool custom_pin_in_use(uint8_t pin)
{
    // no pins are used
    return false;
}

void custom_get_sensors(JsonDocument& doc)
{
    // Unused
}

/* Receive custom topic & payload messages */
void custom_topic_payload(const char* topic, const char* payload, uint8_t source)
{
    bool update = strlen(payload) > 0;
    LOG_INFO(TAG_CUSTOM, "Handling custom message: %s => %s", topic, payload);

    if(update) {
        if(!strcmp(topic, "fanspeed")) {
            fanspeed = atoi(payload);
        } else if(!strcmp(topic, "fanangle")) {
            fanangle = atoi(payload);
        }

    } else {
        char buffer[8] = "";

        if(!strcmp(topic, "fanspeed")) {
            snprintf_P(buffer, sizeof(buffer), PSTR("%d"), fanspeed);
            dispatch_state_subtopic("fanspeed", buffer);
        }
        if(!strcmp(topic, "fanangle")) {
            snprintf_P(buffer, sizeof(buffer), PSTR("%d"), fanangle);
            dispatch_state_subtopic("fanangle", buffer);
        }
    }

    // LOG_VERBOSE(TAG_CUSTOM, "Handled custom message: %s => %s", topic, payload);
}

void custom_state_subtopic(const char* subtopic, const char* payload){
    // Not used
}

#endif // HASP_USE_CUSTOM

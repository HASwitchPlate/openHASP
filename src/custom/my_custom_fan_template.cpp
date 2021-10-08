/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "hasplib.h"

#if defined(HASP_USE_CUSTOM) && false // <-- set this to true in your code

#include "custom/my_custom.h"

uint16_t fanspeed       = 0;
uint16_t fanangle       = 450;
unsigned long prev_loop = 0;

static void my_rotate_fan()
{
    uint8_t page = 1;
    uint8_t id   = 150;

    lv_obj_t* fan = hasp_find_obj_from_page_id(page, id);
    if(!fan) return; // object doesn't exist

    uint16_t angle = lv_img_get_angle(fan) + fanangle;
    lv_img_set_angle(fan, angle % 3600);
}

void custom_setup()
{
    // Initialization code here
    randomSeed(millis());
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
    Serial.print("#");
}

void custom_every_5seconds()
{
    Log.verbose(TAG_CUSTOM, "5 seconds have passsed...");
    dispatch_state_subtopic("my_sensor", "{\"test\":123}");
}

bool custom_pin_in_use(uint pin)
{
    if(pin == 1024) return true; // fictuous used pin

    // otherwise the pin is not used
    return false;
}

void custom_get_sensors(JsonDocument& doc)
{
    /* Sensor Name */
    JsonObject sensor = doc.createNestedObject(F("Custom"));

    /* Key-Value pair of the sensor value */
    sensor[F("Random")] = random(256);
}

/* Receive custom topic & payload messages */
void custom_topic_payload(const char* topic, const char* payload, uint8_t source)
{
    bool update = strlen(payload) > 0;
    Log.notice(TAG_CUSTOM, "Handling custom message: %s => %s", topic, payload);

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

    // Log.verbose(TAG_CUSTOM, "Handled custom message: %s => %s", topic, payload);
}

#endif // HASP_USE_CUSTOM
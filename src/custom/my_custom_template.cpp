/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

// USAGE: - Copy this file and rename it to my_custom.cpp
//        - Change false to true on line 9

#include "hasplib.h"

#if defined(HASP_USE_CUSTOM) && HASP_USE_CUSTOM > 0 && false // <-- set this to true in your code

#include "hasp_debug.h"

void custom_setup()
{
    // Initialization code here
    randomSeed(millis());
}

void custom_loop()
{
    // Non-blocking code here, this should execute very fast!
}

void custom_every_second()
{
    Serial.print("#");
}

void custom_every_5seconds()
{
    LOG_VERBOSE(TAG_CUSTOM, "5 seconds have passsed...");
    dispatch_state_subtopic("my_sensor","{\"test\":123}");
}

bool custom_pin_in_use(uint8_t pin)
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
    sensor[F("Random")] = HASP_RANDOM(256);
}

void custom_topic_payload(const char* topic, const char* payload, uint8_t source){
    // Not used
}

void custom_state_subtopic(const char* subtopic, const char* payload){
    // Not used
}


#endif // HASP_USE_CUSTOM
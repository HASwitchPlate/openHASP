/* MIT License - Copyright (c) 2019-2022 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

// USAGE: - Copy this file and rename it to my_custom.h
//        - uncomment in your user_config_override.h the line containing #define HASP_USE_CUSTOM 1
//

#ifndef HASP_CUSTOM_H
#define HASP_CUSTOM_H

#include "hasplib.h"
#if defined(HASP_USE_CUSTOM)

/* This function is called at boot */
void custom_setup();

/* This function is called every itteration of the main loop */
void custom_loop();

/* This function is called every second */
void custom_every_second();

/* This function is called every 5 seconds */
void custom_every_5seconds();

/* return true if the pin used by the custom code */
bool custom_pin_in_use(uint8_t pin);

/* Add a key which defines a JsonObject to add to the sensor JSON output */
void custom_get_sensors(JsonDocument& doc);

/* Receive custom topic & payload messages */
void custom_topic_payload(const char* topic, const char* payload, uint8_t source);

#endif // HASP_USE_CUSTOM

#endif // HASP_CUSTOM_H

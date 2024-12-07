/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

// USAGE: - Copy this file and rename it to my_custom.h
//        - uncomment in your user_config_override.h the line containing #define HASP_USE_CUSTOM 1
//

#ifndef HASP_CUSTOM_H
#define HASP_CUSTOM_H

#include "hasplib.h"
#if defined(HASP_USE_CUSTOM) && HASP_USE_CUSTOM > 0

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

/* Get notified when a state message is sent out */
/* Can be used to send state changes through other means then MQTT, e.g. Serial2 */
/* https://github.com/HASwitchPlate/openHASP/issues/611 */
void custom_state_subtopic(const char* subtopic, const char* payload);

#endif // HASP_USE_CUSTOM

#endif // HASP_CUSTOM_H

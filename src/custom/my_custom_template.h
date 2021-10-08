/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

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
bool custom_pin_in_use(uint pin);

/* Add a key which defines a JsonObject to add to the sensor JSON document */
void custom_get_sensors(JsonDocument& doc);

#endif // HASP_USE_CUSTOM

#endif // HASP_CUSTOM_H
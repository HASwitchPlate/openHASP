/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_NVS_H
#define HASP_NVS_H

#ifdef ESP32

#include "hasplib.h"
#include <Preferences.h>

void nvs_setup();
bool nvs_user_begin(Preferences& preferences, const char* key, bool readonly);
bool nvs_clear_user_config();
bool nvsUpdateString(Preferences& preferences, const char* key, JsonVariant value);
bool nvsUpdateUInt(Preferences& preferences, const char* key, JsonVariant value);
bool nvsUpdateUShort(Preferences& preferences, const char* key, JsonVariant value);

#endif // ESP32

#endif // HASP_NVS_H
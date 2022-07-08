#ifdef ESP32

#include "hasplib.h"
#include "hasp_nvs.h"

bool nvs_clear_user_config()
{
    const char* name[8] = {"time", "ota"};
    Preferences preferences;

    for(int i = 0; i < 2; i++) {
        preferences.begin(name[i], false);
        preferences.clear();
        preferences.end();
    }

    return true;
}

bool nvsUpdateString(Preferences& preferences, const char* key, JsonVariant value)
{
    bool changed    = false;
    const char* val = value.as<const char*>();

    if(!value.isNull()) {                                            // Json key exists
        if(preferences.isKey(key)) {                                 // Nvs key exists
            changed = preferences.getString(key, "") != String(val); // Value changed
        } else
            changed = true; // Nvs key doesnot exist, create it
        if(changed) {
            size_t len = preferences.putString(key, val);
            LOG_DEBUG(TAG_NVS, F(D_BULLET "Wrote %s => %s (%d bytes)"), key, val, len);
        }
    }

    return changed;
}

bool nvsUpdateUInt(Preferences& preferences, const char* key, JsonVariant value)
{
    bool changed = false;
    uint32_t val = value.as<uint32_t>();

    if(!value.isNull()) {                                 // Json key exists
        if(preferences.isKey(key)) {                      // Nvs key exists
            changed = preferences.getUInt(key, 0) != val; // Value changed
        } else
            changed = true; // Nvs key doesnot exist, create it
        if(changed) {
            size_t len = preferences.putUInt(key, val);
            LOG_DEBUG(TAG_TIME, F(D_BULLET "Wrote %s => %d"), key, val);
        }
    }

    return changed;
}

#endif
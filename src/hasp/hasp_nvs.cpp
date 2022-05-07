#ifdef ESP32

#include "hasplib.h"
#include "hasp_nvs.h"

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
            LOG_VERBOSE(TAG_TIME, F(D_BULLET "Wrote %s => %s (%d bytes)"), key, val, len);
        }
    }

    return changed;
}

#endif
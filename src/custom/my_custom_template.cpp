/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "hasplib.h"
#include "custom/my_custom.h"

#if HASP_USE_CUSTOM > 0

void custom_setup()
{
    // Initialization code here
    randomSeed(millis());
}

void custom_loop()
{
    // Non-blocking code here, this should execure very fast!
}

void custom_every_second()
{
    Serial.print("#");
}

void custom_every_5seconds()
{
    Log.verbose(TAG_CUSTOM, "5 seconds have passsed...");
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

#endif // HASP_USE_CUSTOM
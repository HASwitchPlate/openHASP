#include "Arduino.h"
#include "ArduinoLog.h"
#include "AceButton.h"

#include "hasp_conf.h"
#include "lv_conf.h"

#include "hasp_button.h"
#include "hasp_dispatch.h"

using namespace ace_button;
AceButton * button[HASP_NUM_INPUTS]; // Connect your button between pin 2 and GND

void handleEvent(AceButton *, uint8_t, uint8_t);

void buttonSetup(void)
{
    // button[0] = new Button(2);
    button[1] = new AceButton(3, HIGH, 1);
    button[2] = new AceButton(4, HIGH, 2);

    ButtonConfig * buttonConfig = ButtonConfig::getSystemButtonConfig();
    buttonConfig->setEventHandler(handleEvent);

    // Features
    buttonConfig->setFeature(ButtonConfig::kFeatureClick);
    buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
    buttonConfig->setFeature(ButtonConfig::kFeatureRepeatPress);
    // buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);
    // buttonConfig->setFeature(ButtonConfig::kFeatureSuppressClickBeforeDoubleClick);

    // Delays
    buttonConfig->setClickDelay(LV_INDEV_DEF_LONG_PRESS_TIME);
    buttonConfig->setDoubleClickDelay(LV_INDEV_DEF_LONG_PRESS_TIME);
    buttonConfig->setLongPressDelay(LV_INDEV_DEF_LONG_PRESS_TIME);
    buttonConfig->setRepeatPressDelay(LV_INDEV_DEF_LONG_PRESS_TIME);
    buttonConfig->setRepeatPressInterval(LV_INDEV_DEF_LONG_PRESS_REP_TIME);

    Log.verbose(F("BTNS: setup(): ready"));
}

void buttonLoop(void)
{

    // Should be called every 4-5ms or faster, for the default debouncing time
    // of ~20ms.
    for(uint8_t i = 0; i < (sizeof button / sizeof *button); i++) {
        if(button[i]) button[i]->check();
    }
}

// The event handler for the button.
void handleEvent(AceButton * button, uint8_t eventType, uint8_t buttonState)
{
    char buffer[128];

    // Print out a message for all events.
    /* Serial.print(F("handleEvent(): eventType: "));
    Serial.print(eventType);
    Serial.print(F("; buttonState: "));
    Serial.println(buttonState); */

    switch(eventType) {
        case AceButton::kEventPressed:
            memcpy_P(buffer, PSTR("DOWN"), sizeof(buffer));
            break;
        case AceButton::kEventClicked:
            memcpy_P(buffer, PSTR("SHORT"), sizeof(buffer));
            break;
        case AceButton::kEventDoubleClicked:
            memcpy_P(buffer, PSTR("DOUBLE"), sizeof(buffer));
            break;
        case AceButton::kEventLongPressed:
            memcpy_P(buffer, PSTR("LONG"), sizeof(buffer));
            break;
        case AceButton::kEventRepeatPressed:
            memcpy_P(buffer, PSTR("HOLD"), sizeof(buffer));
            break;
        case AceButton::kEventReleased:
            memcpy_P(buffer, PSTR("UP"), sizeof(buffer));
            break;
    }
    dispatchButton(button->getId(), buffer);
}
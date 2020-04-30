#include "Arduino.h"
#include "ArduinoLog.h"
#include "AceButton.h"

#include "hasp_conf.h"
#include "lv_conf.h"

#include "hasp_mqtt.h" // testing memory consumption
#include "hasp_button.h"
#include "hasp_dispatch.h"

using namespace ace_button;
static AceButton * button[HASP_NUM_INPUTS]; // Connect your button between pin 2 and GND

static void button_event_cb(AceButton * button, uint8_t eventType, uint8_t buttonState)
{
    char buffer[8];
    switch(eventType) {
        case 0: // AceButton::kEventPressed:
            memcpy_P(buffer, PSTR("DOWN"), sizeof(buffer));
            break;
        case 2: // AceButton::kEventClicked:
            memcpy_P(buffer, PSTR("SHORT"), sizeof(buffer));
            break;
        // case AceButton::kEventDoubleClicked:
        //    memcpy_P(buffer, PSTR("DOUBLE"), sizeof(buffer));
        //    break;
        case 4: // AceButton::kEventLongPressed:
            memcpy_P(buffer, PSTR("LONG"), sizeof(buffer));
            break;
        case 5:     // AceButton::kEventRepeatPressed:
            return; // Fix needed for switches
            memcpy_P(buffer, PSTR("HOLD"), sizeof(buffer));
            break;
        case 1: // AceButton::kEventReleased:
            memcpy_P(buffer, PSTR("UP"), sizeof(buffer));
            break;
    }
        Log.verbose(F("BTNS: setup(): ready"));

    dispatch_button(button->getId(), buffer);
}

void buttonSetup(void)
{
    ButtonConfig * buttonConfig = ButtonConfig::getSystemButtonConfig();
    buttonConfig->setEventHandler(button_event_cb);

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

    // button[0] = new Button(2);
#if defined(ARDUINO_ARCH_ESP8266)
    button[1] = new AceButton(3, HIGH, 1);
#else
    //button[1] = new AceButton(3, HIGH, 1);
#endif
    pinMode (PC13, INPUT_PULLUP);
    button[0] = new AceButton(buttonConfig, PC13, HIGH, 0);

    pinMode (PA0, INPUT_PULLUP);
    button[1] = new AceButton(buttonConfig, PA0, HIGH, 1);

    pinMode (PD15, INPUT);
    button[2] = new AceButton(buttonConfig, PD15, HIGH, 2);

    Log.verbose(F("BTNS: setup(): ready"));


}

void IRAM_ATTR buttonLoop(void)
{
    // Should be called every 4-5ms or faster, for the default debouncing time
    // of ~20ms.
    for(uint8_t i = 0; i < HASP_NUM_INPUTS; i++) {
        if(button[i]) button[i]->check();
    }
}

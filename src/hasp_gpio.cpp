#include "Arduino.h"
#include "ArduinoLog.h"

#include "AceButton.h"
#include "lv_conf.h" // For timing defines

#include "hasp_conf.h"
#include "hasp_gpio.h"
#include "hasp_config.h"
#include "hasp_dispatch.h"
#include "hasp.h"

uint8_t gpioUsedInputCount = 0;

using namespace ace_button;
static AceButton * button[HASP_NUM_INPUTS];

struct hasp_gpio_config_t
{
    uint8_t pin;           // pin number
    uint8_t group;         // groupid
    uint8_t type;          // switch, button, ...
    uint8_t gpio_function; // INPUT, OUTPUT, PULLUP, etc
};

hasp_gpio_config_t gpioConfig[HASP_NUM_GPIO_CONFIG];

#if defined(ARDUINO_ARCH_ESP32)
class TouchConfig : public ButtonConfig {
  public:
    TouchConfig();

  protected:
    // Number of iterations to sample the capacitive switch. Higher number
    // provides better smoothing but increases the time taken for a single read.
    static const uint8_t kSamples = 10;

    // The threshold value which is considered to be a "touch" on the switch.
    static const long kTouchThreshold = 70;

    int readButton(uint8_t pin) override
    {
        // long total =  mSensor.capacitiveSensor(kSamples);
        return (touchRead(pin) > kTouchThreshold) ? LOW : HIGH;
    }
};

TouchConfig touchConfig();
#endif

static void gpio_event_handler(AceButton * button, uint8_t eventType, uint8_t buttonState)
{
    uint8_t eventid;
    char buffer[16];
    switch(eventType) {
        case AceButton::kEventPressed:
            eventid = HASP_EVENT_DOWN;
            memcpy_P(buffer, PSTR("DOWN"), sizeof(buffer));
            break;
        case 2: // AceButton::kEventClicked:
            eventid = HASP_EVENT_SHORT;
            memcpy_P(buffer, PSTR("SHORT"), sizeof(buffer));
            break;
        case AceButton::kEventDoubleClicked:
            eventid = HASP_EVENT_DOUBLE;
            memcpy_P(buffer, PSTR("DOUBLE"), sizeof(buffer));
            break;
        case AceButton::kEventLongPressed:
            eventid = HASP_EVENT_LONG;
            memcpy_P(buffer, PSTR("LONG"), sizeof(buffer));
            break;
        case AceButton::kEventRepeatPressed:
            // return; // Fix needed for switches
            eventid = HASP_EVENT_HOLD;
            memcpy_P(buffer, PSTR("HOLD"), sizeof(buffer));
            break;
        case AceButton::kEventReleased:
            eventid = HASP_EVENT_UP;
            memcpy_P(buffer, PSTR("UP"), sizeof(buffer));
            break;
        default:
            eventid = HASP_EVENT_LOST;
            memcpy_P(buffer, PSTR("UNKNOWN"), sizeof(buffer));
    }
    dispatch_button(button->getId(), buffer);
    dispatch_send_group_event(gpioConfig[button->getId()].group, eventid, true);
}

void aceButtonSetup(void)
{
    ButtonConfig * buttonConfig = ButtonConfig::getSystemButtonConfig();
    buttonConfig->setEventHandler(gpio_event_handler);

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
}

void IRAM_ATTR gpioLoop(void)
{
    // Should be called every 4-5ms or faster, for the default debouncing time of ~20ms.
    for(uint8_t i = 0; i < gpioUsedInputCount; i++) {
        if(button[i]) button[i]->check();
    }
}

void gpioAddButton(uint8_t pin, uint8_t input_mode, uint8_t default_state, uint8_t index)
{
    uint8_t i;
    for(i = 0; i < HASP_NUM_INPUTS; i++) {

        if(!button[i]) {
            button[i] = new AceButton(pin, default_state, index);
            // button[i]->init(pin, default_state, index);

            if(button[i]) {
                pinMode(pin, input_mode);

                ButtonConfig * buttonConfig = button[i]->getButtonConfig();
                buttonConfig->setEventHandler(gpio_event_handler);
                buttonConfig->setFeature(ButtonConfig::kFeatureClick);
                buttonConfig->clearFeature(ButtonConfig::kFeatureDoubleClick);
                buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
                buttonConfig->clearFeature(ButtonConfig::kFeatureRepeatPress);
                buttonConfig->clearFeature(
                    ButtonConfig::kFeatureSuppressClickBeforeDoubleClick); // Causes annoying pauses

                Log.verbose(F("GPIO: Button%d created on pin %d (index %d) mode %d default %d"), i, pin, index,
                            input_mode, default_state);
                gpioUsedInputCount = i + 1;
                return;
            }
        }
    }
    Log.error(F("GPIO: Failed to create Button%d pin %d (index %d). All %d slots available are in use!"), i, pin, index,
              HASP_NUM_INPUTS);
}

void gpioAddTouchButton(uint8_t pin, uint8_t input_mode, uint8_t default_state, uint8_t index)
{
    uint8_t i;
    for(i = 0; i < HASP_NUM_INPUTS; i++) {

        if(!button[i]) {
            button[i] = new AceButton(pin, default_state, index);

            if(button[i]) {
                pinMode(pin, input_mode);

                ButtonConfig * buttonConfig = button[i]->getButtonConfig();
                buttonConfig->setEventHandler(gpio_event_handler);
                buttonConfig->setFeature(ButtonConfig::kFeatureClick);
                buttonConfig->clearFeature(ButtonConfig::kFeatureDoubleClick);
                buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
                buttonConfig->clearFeature(ButtonConfig::kFeatureRepeatPress);
                buttonConfig->clearFeature(
                    ButtonConfig::kFeatureSuppressClickBeforeDoubleClick); // Causes annoying pauses

                Log.verbose(F("GPIO: Button%d created on pin %d (index %d) mode %d default %d"), i, pin, index,
                            input_mode, default_state);
                gpioUsedInputCount = i + 1;
                return;
            }
        }
    }
    Log.error(F("GPIO: Failed to create Button%d pin %d (index %d). All %d slots available are in use!"), i, pin, index,
              HASP_NUM_INPUTS);
}

void gpioSetup()
{
    aceButtonSetup();

    // return;

#if defined(ARDUINO_ARCH_ESP8266)
    gpioConfig[0] = {D2, 7, HASP_GPIO_BUTTON, INPUT_PULLUP};
    gpioConfig[1] = {D1, 7, HASP_GPIO_LED, OUTPUT};

// gpioAddButton(D2, INPUT_PULLUP, HIGH, 1);
// pinMode(D1, OUTPUT);
#endif

#if defined(ARDUINO_ARCH_ESP32)
       gpioConfig[0] = {D2, 0, HASP_GPIO_SWITCH, INPUT};
       gpioConfig[1] = {D1, 1, HASP_GPIO_RELAY, OUTPUT};

// gpioAddButton(D2, INPUT, HIGH, 1);
// pinMode(D1, OUTPUT);
#endif

    /*
    #if defined(ARDUINO_ARCH_ESP8266)
        pinMode(D1, OUTPUT);
        pinMode(D2, INPUT_PULLUP);
    #endif
    #if defined(STM32F4xx)
        pinMode(HASP_OUTPUT_PIN, OUTPUT);
        pinMode(HASP_INPUT_PIN, INPUT);
    #endif
    */

    for(uint8_t i = 0; i < HASP_NUM_GPIO_CONFIG; i++) {
        uint8_t input_mode;
        switch(gpioConfig[i].gpio_function) {
            case OUTPUT:
                input_mode = OUTPUT;
                break;
            case INPUT_PULLUP:
                input_mode = INPUT_PULLUP;
                break;
#ifndef ARDUINO_ARCH_ESP8266
            case INPUT_PULLDOWN:
                input_mode = INPUT_PULLDOWN;
                break;
#endif
            default:
                input_mode = INPUT;
        }

        switch(gpioConfig[i].type) {
            case HASP_GPIO_SWITCH:
            case HASP_GPIO_BUTTON:
                gpioAddButton(gpioConfig[i].pin, input_mode, HIGH, i);
                break;
            case HASP_GPIO_SWITCH_INVERTED:
            case HASP_GPIO_INPUT_BUTTON_INVERTED:
                gpioAddButton(gpioConfig[i].pin, input_mode, LOW, i);
                break;

            case HASP_GPIO_RELAY:
            case HASP_GPIO_RELAY_INVERTED:
            case HASP_GPIO_LED:
            case HASP_GPIO_LED_INVERTED:
                pinMode(gpioConfig[i].pin, OUTPUT);
                break;

            case HASP_GPIO_PWM:
            case HASP_GPIO_PWM_INVERTED:
                // case HASP_GPIO_BACKLIGHT:
                pinMode(gpioConfig[i].pin, OUTPUT);
#if defined(ARDUINO_ARCH_ESP32)
                // configure LED PWM functionalitites
                ledcSetup(gpioConfig[i].group, 20000, 10);
                // attach the channel to the GPIO to be controlled
                ledcAttachPin(gpioConfig[i].pin, gpioConfig[i].group);
#endif
                break;
        }
    }
}

void gpio_set_group_outputs(uint8_t groupid, uint8_t eventid)
{
    bool state = dispatch_get_event_state(eventid);
    for(uint8_t i = 0; i < HASP_NUM_GPIO_CONFIG; i++) {
        if(gpioConfig[i].group == groupid) {
            switch(gpioConfig[i].type) {
                case HASP_GPIO_RELAY:
                case HASP_GPIO_LED:
                    digitalWrite(gpioConfig[i].pin, state ? HIGH : LOW);
                    break;
                case HASP_GPIO_RELAY_INVERTED:
                case HASP_GPIO_LED_INVERTED:
                    digitalWrite(gpioConfig[i].pin, state ? LOW : HIGH);
                    break;
#if defined(ARDUINO_ARCH_ESP32)
                case HASP_GPIO_PWM:
                    ledcWrite(groupid, map(state, 0, 1, 0, 1023)); // ledChannel and value
                    break;
                case HASP_GPIO_PWM_INVERTED:
                    ledcWrite(groupid, map(!state, 0, 1, 0, 1023)); // ledChannel and value
                    break;
#else
                case HASP_GPIO_PWM:
                    analogWrite(gpioConfig[i].pin, map(state, 0, 1, 0, 1023));
                    break;
                case HASP_GPIO_PWM_INVERTED:
                    analogWrite(gpioConfig[i].pin, map(!state, 0, 1, 0, 1023));
                    break;
#endif
                default:;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool gpioGetConfig(const JsonObject & settings)
{
    bool changed = false;

    /* Check Gpio array has changed */
    JsonArray array = settings[FPSTR(F_GPIO_CONFIG)].as<JsonArray>();
    uint8_t i       = 0;
    for(JsonVariant v : array) {
        if(i < HASP_NUM_GPIO_CONFIG) {
            uint32_t cur_val = gpioConfig[i].pin | (gpioConfig[i].group << 8) | (gpioConfig[i].type << 16) |
                               (gpioConfig[i].gpio_function << 24);
            Log.verbose(F("GPIO CONF: %d: %d <=> %d"), i, cur_val, v.as<uint32_t>());

            if(cur_val != v.as<uint32_t>()) changed = true;
            v.set(cur_val);
        } else {
            changed = true;
        }
        i++;
    }

    /* Build new Gpio array if the count is not correct */
    if(i != HASP_NUM_GPIO_CONFIG) {
        array = settings[FPSTR(F_GPIO_CONFIG)].to<JsonArray>(); // Clear JsonArray
        for(uint8_t i = 0; i < HASP_NUM_GPIO_CONFIG; i++) {
            uint32_t cur_val = gpioConfig[i].pin | (gpioConfig[i].group << 8) | (gpioConfig[i].type << 16) |
                               (gpioConfig[i].gpio_function << 24);
            array.add(cur_val);
        }
        changed = true;
    }

    if(changed) configOutput(settings);
    return changed;
}

/** Set GPIO Configuration.
 *
 * Read the settings from json and sets the application variables.
 *
 * @note: data pixel should be formated to uint32_t RGBA. Imagemagick requirements.
 *
 * @param[in] settings    JsonObject with the config settings.
 **/
bool gpioSetConfig(const JsonObject & settings)
{
    configOutput(settings);
    bool changed = false;

    if(!settings[FPSTR(F_GPIO_CONFIG)].isNull()) {
        bool status = false;
        int i       = 0;

        JsonArray array = settings[FPSTR(F_GPIO_CONFIG)].as<JsonArray>();
        for(JsonVariant v : array) {
            uint32_t new_val = v.as<uint32_t>();

            if(i < HASP_NUM_GPIO_CONFIG) {
                uint32_t cur_val = gpioConfig[i].pin | (gpioConfig[i].group << 8) | (gpioConfig[i].type << 16) |
                                   (gpioConfig[i].gpio_function << 24);
                if(cur_val != new_val) status = true;

                gpioConfig[i].pin           = new_val & 0xFF;
                gpioConfig[i].group         = new_val >> 8 & 0xFF;
                gpioConfig[i].type          = new_val >> 16 & 0xFF;
                gpioConfig[i].gpio_function = new_val >> 24 & 0xFF;
            }
            i++;
        }
        changed |= status;
    }

    return changed;
}

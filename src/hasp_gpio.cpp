#include "Arduino.h"
#include "ArduinoLog.h"

#include "AceButton.h"
#include "lv_conf.h" // For timing defines

#include "hasp_conf.h"
#include "hasp_gpio.h"
#include "hasp_dispatch.h"

#define HASP_NUM_GPIO_CONFIG 5

uint8_t gpioUsedInputCount = 0;
uint16_t gpioConfig[HASP_NUM_GPIO_CONFIG];

using namespace ace_button;
static AceButton * button[HASP_NUM_INPUTS];

struct hasp_gpio_config_t
{
    const uint8_t pin;
    const uint8_t group;
    const uint8_t io_mode;
    bool default_state;
};

// An array of button pins, led pins, and the led states. Cannot be const
// because ledState is mutable.
hasp_gpio_config_t gpioConfig2[HASP_NUM_GPIO_CONFIG] = {
    {2, 8, INPUT, LOW}, {3, 9, OUTPUT, LOW}, {4, 10, INPUT, HIGH}, {5, 11, OUTPUT, LOW}, {6, 12, INPUT, LOW},
};

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

static void gpio_event_cb(AceButton * button, uint8_t eventType, uint8_t buttonState)
{
    char buffer[16];
    switch(eventType) {
        case 0: // AceButton::kEventPressed:
            memcpy_P(buffer, PSTR("DOWN"), sizeof(buffer));
            break;
        case 2: // AceButton::kEventClicked:
            memcpy_P(buffer, PSTR("SHORT"), sizeof(buffer));
            break;
        case AceButton::kEventDoubleClicked:
            memcpy_P(buffer, PSTR("DOUBLE"), sizeof(buffer));
            break;
        case 4: // AceButton::kEventLongPressed:
            memcpy_P(buffer, PSTR("LONG"), sizeof(buffer));
            break;
        case 5: // AceButton::kEventRepeatPressed:
            // return; // Fix needed for switches
            memcpy_P(buffer, PSTR("HOLD"), sizeof(buffer));
            break;
        case 1: // AceButton::kEventReleased:
            memcpy_P(buffer, PSTR("UP"), sizeof(buffer));
            break;
        default:
            memcpy_P(buffer, PSTR("UNKNOWN"), sizeof(buffer));
    }
    dispatch_button(button->getId(), buffer);
}

void aceButtonSetup(void)
{
    ButtonConfig * buttonConfig = ButtonConfig::getSystemButtonConfig();
    buttonConfig->setEventHandler(gpio_event_cb);

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

void gpioAddButton(uint8_t pin, uint8_t input_mode, uint8_t default_state, uint8_t channel)
{

    uint8_t i;
    for(i = 0; i < HASP_NUM_INPUTS; i++) {

        if(!button[i]) {
            button[i] = new AceButton(pin, default_state, channel);
            // button[i]->init(pin, default_state, channel);

            if(button[i]) {
                pinMode(pin, input_mode);

                ButtonConfig * buttonConfig = button[i]->getButtonConfig();
                buttonConfig->setEventHandler(gpio_event_cb);
                buttonConfig->setFeature(ButtonConfig::kFeatureClick);
                buttonConfig->clearFeature(ButtonConfig::kFeatureDoubleClick);
                buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
                buttonConfig->clearFeature(ButtonConfig::kFeatureRepeatPress);
                buttonConfig->clearFeature(
                    ButtonConfig::kFeatureSuppressClickBeforeDoubleClick); // Causes annoying pauses

                Log.verbose(F("GPIO: Button%d created on pin %d (channel %d) mode %d default %d"), i, pin, channel,
                            input_mode, default_state);
                gpioUsedInputCount = i + 1;
                return;
            }
        }
    }
    Log.error(F("GPIO: Failed to create Button%d pin %d (channel %d). All %d slots available are in use!"), i, pin,
              channel, HASP_NUM_INPUTS);
}

void gpioAddTouchButton(uint8_t pin, uint8_t input_mode, uint8_t default_state, uint8_t channel)
{
    uint8_t i;
    for(i = 0; i < HASP_NUM_INPUTS; i++) {

        if(!button[i]) {
            button[i] = new AceButton();

            if(button[i]) {
                pinMode(pin, input_mode);

                ButtonConfig * buttonConfig = button[i]->getButtonConfig();
                buttonConfig->setEventHandler(gpio_event_cb);
                buttonConfig->setFeature(ButtonConfig::kFeatureClick);
                buttonConfig->clearFeature(ButtonConfig::kFeatureDoubleClick);
                buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
                buttonConfig->clearFeature(ButtonConfig::kFeatureRepeatPress);
                buttonConfig->clearFeature(
                    ButtonConfig::kFeatureSuppressClickBeforeDoubleClick); // Causes annoying pauses

                Log.verbose(F("GPIO: Button%d created on pin %d (channel %d) mode %d default %d"), i, pin, channel,
                            input_mode, default_state);
                gpioUsedInputCount = i + 1;
                return;
            }
        }
    }
    Log.error(F("GPIO: Failed to create Button%d pin %d (channel %d). All %d slots available are in use!"), i, pin,
              channel, HASP_NUM_INPUTS);
}

void gpioSetup()
{
    aceButtonSetup();

    // gpioConfig[0] = PD15 * 256 + 5 + (INPUT << 3);
#if defined(ARDUINO_ARCH_ESP8266)
    gpioAddButton(D2, INPUT_PULLUP, HIGH, 1);
    pinMode(D1, OUTPUT);
#endif

#if defined(ARDUINO_ARCH_ESP32)
    // gpioAddButton( D2, INPUT, HIGH, 1);
    // pinMode(D1, OUTPUT);
#endif

    for(uint8_t i = 0; i < HASP_NUM_GPIO_CONFIG; i++) {
        uint8_t pin        = (gpioConfig[i] >> 8) & 0xFF;
        uint8_t channel    = gpioConfig[i] & 0b111;       // 3bit
        uint8_t input_mode = (gpioConfig[i] >> 3) & 0b11; // 2bit gpio mode
        // uint8_t input_mode    = gpioConfig[i].io_mode
        uint8_t gpiotype      = (gpioConfig[i] >> 5) & 0b111; // 3bit
        uint8_t default_state = gpioConfig[i] & 0b1;          // 1bit: 0=LOW, 1=HIGH

        switch(input_mode) {
            case 1:
                input_mode = OUTPUT;
                break;
            case 2:
                input_mode = INPUT_PULLUP;
                break;
#ifndef ARDUINO_ARCH_ESP8266
            case 3:
                input_mode = INPUT_PULLDOWN;
                break;
#endif
            default:
                input_mode = INPUT;
        }

        switch(gpiotype) {
            case HASP_GPIO_SWITCH:
            case HASP_GPIO_BUTTON:
                // gpioAddButton(gpioConfig[i].io_mode.pin, input_mode, gpioConfig[i].default_state,
                // gpioConfig[i].group);
                break;

            case HASP_GPIO_RELAY:
                pinMode(pin, OUTPUT);
                break;

            // case HASP_GPIO_LED:
            case HASP_GPIO_PWM:
            case HASP_GPIO_BACKLIGHT:
                pinMode(pin, OUTPUT);
#if defined(ARDUINO_ARCH_ESP32)
                // configure LED PWM functionalitites
                ledcSetup(channel, 20000, 10);
                // attach the channel to the GPIO to be controlled
                ledcAttachPin(pin, channel);
#endif
                break;
        }
    }

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
}
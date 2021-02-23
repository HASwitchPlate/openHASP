/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "AceButton.h"
#include "lv_conf.h" // For timing defines

#include "hasp_conf.h"
#include "hasp_gpio.h"
#include "hasp_config.h"

#include "hasp/hasp_dispatch.h"
#include "hasp/hasp.h"

#ifdef ARDUINO_ARCH_ESP8266
#define INPUT_PULLDOWN INPUT
#endif

uint8_t gpioUsedInputCount = 0;

using namespace ace_button;
static AceButton* button[HASP_NUM_INPUTS];

// An array of button pins, led pins, and the led states. Cannot be const
// because ledState is mutable.
hasp_gpio_config_t gpioConfig[HASP_NUM_GPIO_CONFIG] = {
    //    {2, 8, INPUT, LOW}, {3, 9, OUTPUT, LOW}, {4, 10, INPUT, HIGH}, {5, 11, OUTPUT, LOW}, {6, 12, INPUT, LOW},
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

static void gpio_event_handler(AceButton* button, uint8_t eventType, uint8_t buttonState)
{
    uint8_t btnid = button->getId();
    uint8_t eventid;
    bool state = false;
    switch(eventType) {
        case AceButton::kEventPressed:
            if(gpioConfig[btnid].type == HASP_GPIO_SWITCH || gpioConfig[btnid].type == HASP_GPIO_SWITCH_INVERTED) {
                eventid = HASP_EVENT_ON;
            } else {
                eventid = HASP_EVENT_DOWN;
            }
            state = true;
            break;
        case 2: // AceButton::kEventClicked:
            eventid = HASP_EVENT_SHORT;
            break;
        // case AceButton::kEventDoubleClicked:
        //     eventid = HASP_EVENT_DOUBLE;
        //     break;
        case AceButton::kEventLongPressed:
            eventid = HASP_EVENT_LONG;
            // state = true; // do not repeat DOWN + LONG
            break;
        // case AceButton::kEventRepeatPressed:
        //     eventid = HASP_EVENT_HOLD;
        //     state = true; // do not repeat DOWN + LONG + HOLD
        //     break;
        case AceButton::kEventReleased:
            if(gpioConfig[btnid].type == HASP_GPIO_SWITCH || gpioConfig[btnid].type == HASP_GPIO_SWITCH_INVERTED) {
                eventid = HASP_EVENT_OFF;
            } else {
                eventid = HASP_EVENT_UP;
            }
            break;
        default:
            eventid = HASP_EVENT_LOST;
    }

    dispatch_gpio_input_event(gpioConfig[btnid].pin, gpioConfig[btnid].group, eventid);
    if(eventid != HASP_EVENT_LONG) // do not repeat DOWN + LONG
        dispatch_normalized_group_value(gpioConfig[btnid].group, NORMALIZE(state, 0, 1), NULL);
}

void aceButtonSetup(void)
{
    ButtonConfig* buttonConfig = ButtonConfig::getSystemButtonConfig();
    buttonConfig->setEventHandler(gpio_event_handler);

    // Features
    // buttonConfig->setFeature(ButtonConfig::kFeatureClick);
    // buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
    // buttonConfig->setFeature(ButtonConfig::kFeatureRepeatPress);
    // buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);
    // buttonConfig->setFeature(ButtonConfig::kFeatureSuppressClickBeforeDoubleClick);

    // Delays
    buttonConfig->setClickDelay(LV_INDEV_DEF_LONG_PRESS_TIME);
    buttonConfig->setDoubleClickDelay(LV_INDEV_DEF_LONG_PRESS_TIME);
    buttonConfig->setLongPressDelay(LV_INDEV_DEF_LONG_PRESS_TIME);
    buttonConfig->setRepeatPressDelay(LV_INDEV_DEF_LONG_PRESS_TIME);
    buttonConfig->setRepeatPressInterval(LV_INDEV_DEF_LONG_PRESS_REP_TIME);
}

void gpioLoop(void)
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
            LOG_TRACE(TAG_GPIO, F("Creating Button%d on pin %d (index %d) mode %d default %d"), i, pin, index,
                      input_mode, default_state);

            button[i] = new AceButton(pin, default_state, index);

            if(button[i]) {
                // pinMode(pin, input_mode);

                ButtonConfig* buttonConfig = button[i]->getButtonConfig();
                buttonConfig->setEventHandler(gpio_event_handler);
                buttonConfig->setFeature(ButtonConfig::kFeatureClick);
                buttonConfig->clearFeature(ButtonConfig::kFeatureDoubleClick);
                buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
                // buttonConfig->clearFeature(ButtonConfig::kFeatureRepeatPress);
                buttonConfig->clearFeature(
                    ButtonConfig::kFeatureSuppressClickBeforeDoubleClick); // Causes annoying pauses
                buttonConfig->setFeature(ButtonConfig::kFeatureSuppressAfterClick);

                LOG_INFO(TAG_GPIO, F("Button%d created on pin %d (index %d) mode %d default %d"), i, pin, index,
                         input_mode, default_state);
                gpioUsedInputCount = i + 1;
                return;
            }
        }
    }
    LOG_ERROR(TAG_GPIO, F("Failed to create Button%d pin %d (index %d). All %d slots available are in use!"), i, pin,
              index, HASP_NUM_INPUTS);
}

void gpioAddSwitch(uint8_t pin, uint8_t input_mode, uint8_t default_state, uint8_t index)
{
    uint8_t i;
    for(i = 0; i < HASP_NUM_INPUTS; i++) {

        if(!button[i]) {
            LOG_TRACE(TAG_GPIO, F("Creating Switch%d on pin %d (index %d) mode %d default %d"), i, pin, index,
                      input_mode, default_state);

            button[i] = new AceButton(pin, default_state, index);

            if(button[i]) {
                // pinMode(pin, input_mode);

                ButtonConfig* buttonConfig = button[i]->getButtonConfig();
                buttonConfig->setEventHandler(gpio_event_handler);
                buttonConfig->setFeature(ButtonConfig::kFeatureSuppressAll);

                LOG_INFO(TAG_GPIO, F("Button%d switch on pin %d (index %d) mode %d default %d"), i, pin, index,
                         input_mode, default_state);
                gpioUsedInputCount = i + 1;
                return;
            }
        }
    }
    LOG_ERROR(TAG_GPIO, F("Failed to create Button%d pin %d (index %d). All %d slots available are in use!"), i, pin,
              index, HASP_NUM_INPUTS);
}

void gpioAddTouchButton(uint8_t pin, uint8_t input_mode, uint8_t default_state, uint8_t index)
{
    uint8_t i;
    for(i = 0; i < HASP_NUM_INPUTS; i++) {

        if(!button[i]) {
            button[i] = new AceButton(pin, default_state, index);

            if(button[i]) {
                pinMode(pin, input_mode);

                ButtonConfig* buttonConfig = button[i]->getButtonConfig();
                buttonConfig->setEventHandler(gpio_event_handler);
                buttonConfig->setFeature(ButtonConfig::kFeatureClick);
                buttonConfig->clearFeature(ButtonConfig::kFeatureDoubleClick);
                buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
                buttonConfig->clearFeature(ButtonConfig::kFeatureRepeatPress);
                buttonConfig->clearFeature(
                    ButtonConfig::kFeatureSuppressClickBeforeDoubleClick); // Causes annoying pauses

                LOG_INFO(TAG_GPIO, F("Button%d created on pin %d (index %d) mode %d default %d"), i, pin, index,
                         input_mode, default_state);
                gpioUsedInputCount = i + 1;
                return;
            }
        }
    }
    LOG_ERROR(TAG_GPIO, F("Failed to create Button%d pin %d (index %d). All %d slots available are in use!"), i, pin,
              index, HASP_NUM_INPUTS);
}

void gpioSetup()
{
    aceButtonSetup();

    for(uint8_t i = 0; i < HASP_NUM_GPIO_CONFIG; i++) {
        uint8_t input_mode;
        switch(gpioConfig[i].gpio_function) {
            case OUTPUT:
                input_mode = OUTPUT;
                break;
            case INPUT:
                input_mode = INPUT;
                break;
#ifndef ARDUINO_ARCH_ESP8266
            case INPUT_PULLDOWN:
                input_mode = INPUT_PULLDOWN;
                break;
#endif
            default:
                input_mode = INPUT_PULLUP;
        }

        switch(gpioConfig[i].type) {
            case HASP_GPIO_SWITCH:
                gpioAddSwitch(gpioConfig[i].pin, input_mode, HIGH, i);
                pinMode(gpioConfig[i].pin, INPUT_PULLUP);
                break;
            case HASP_GPIO_BUTTON:
                gpioAddButton(gpioConfig[i].pin, input_mode, HIGH, i);
                pinMode(gpioConfig[i].pin, INPUT_PULLUP);
                break;
            case HASP_GPIO_SWITCH_INVERTED:
                gpioAddSwitch(gpioConfig[i].pin, input_mode, LOW, i);
                pinMode(gpioConfig[i].pin, INPUT_PULLDOWN);
                break;
            case HASP_GPIO_BUTTON_INVERTED:
                gpioAddButton(gpioConfig[i].pin, input_mode, LOW, i);
                pinMode(gpioConfig[i].pin, INPUT_PULLDOWN);
                break;

            case HASP_GPIO_RELAY:
            case HASP_GPIO_RELAY_INVERTED:
                pinMode(gpioConfig[i].pin, OUTPUT);
                break;

            case HASP_GPIO_LED ... HASP_GPIO_LED_CW_INVERTED:
            // case HASP_GPIO_LED_INVERTED:
            case HASP_GPIO_PWM:
            case HASP_GPIO_PWM_INVERTED:
                // case HASP_GPIO_BACKLIGHT:
                pinMode(gpioConfig[i].pin, OUTPUT);
#if defined(ARDUINO_ARCH_ESP32)
                // configure LED PWM functionalitites
                ledcSetup(gpioConfig[i].group, 20000, 12);
                // attach the channel to the GPIO to be controlled
                ledcAttachPin(gpioConfig[i].pin, gpioConfig[i].group);
#endif
                break;
        }
    }
}

/* ********************************* State Setters *************************************** */
void gpio_set_normalized_value(hasp_gpio_config_t gpio, uint16_t state)
{
    switch(gpio.type) {
        case HASP_GPIO_RELAY:
            gpio.val = state >= 0x8000U ? HIGH : LOW;
            digitalWrite(gpio.pin, gpio.val);
            break;
        case HASP_GPIO_RELAY_INVERTED:
            gpio.val = state >= 0x8000U ? LOW : HIGH;
            digitalWrite(gpio.pin, gpio.val);
            break;
        case HASP_GPIO_LED:
        case HASP_GPIO_LED_R:
        case HASP_GPIO_LED_G:
        case HASP_GPIO_LED_B:
        case HASP_GPIO_PWM:
#if defined(ARDUINO_ARCH_ESP32)
            gpio.val = map(state, 0, 0xFFFFU, 0, 4095);
            ledcWrite(gpio.group, gpio.val); // ledChannel and value
#else
            analogWrite(gpio.pin, map(state, 0, 0xFFFFU, 0, 1023));
#endif
            break;
        case HASP_GPIO_LED_INVERTED:
        case HASP_GPIO_LED_R_INVERTED:
        case HASP_GPIO_LED_G_INVERTED:
        case HASP_GPIO_LED_B_INVERTED:
        case HASP_GPIO_PWM_INVERTED:
#if defined(ARDUINO_ARCH_ESP32)
            gpio.val = map(0xFFFFU - state, 0, 0xFFFFU, 0, 4095);
            ledcWrite(gpio.group, gpio.val); // ledChannel and value
#else
            analogWrite(gpio.pin, map(0xFFFFU - state, 0, 0xFFFFU, 0, 1023));
#endif
            break;

        default:
            return;
    }
    LOG_VERBOSE(TAG_GPIO, F(D_BULLET "Group %d - Pin %d = %d"), gpio.group, gpio.pin, gpio.val);
}

// void gpio_set_group_onoff(uint8_t groupid, bool ison)
// {
//     for(uint8_t i = 0; i < HASP_NUM_GPIO_CONFIG; i++) {
//         if(gpioConfig[i].group == groupid) {
//             gpio_set_value(gpioConfig[i], ison ? gpioConfig[i].max : 0);
//         }
//     }
// }

void gpio_set_normalized_group_value(uint8_t groupid, uint16_t state)
{
    // bool state = dispatch_get_event_state(eventid);
    for(uint8_t i = 0; i < HASP_NUM_GPIO_CONFIG; i++) {
        if(gpioConfig[i].group == groupid) {
            gpio_set_normalized_value(gpioConfig[i], state);
        }
    }
}

void gpio_set_moodlight(uint8_t r, uint8_t g, uint8_t b)
{
    for(uint8_t i = 0; i < HASP_NUM_GPIO_CONFIG; i++) {
        switch(gpioConfig[i].type & 0xfe) {
            case HASP_GPIO_LED_R:
                gpio_set_normalized_value(gpioConfig[i], map(r, 0, 0xFF, 0, 0xFFFFU));
                break;
            case HASP_GPIO_LED_G:
                gpio_set_normalized_value(gpioConfig[i], map(g, 0, 0xFF, 0, 0xFFFFU));
                break;
            case HASP_GPIO_LED_B:
                gpio_set_normalized_value(gpioConfig[i], map(b, 0, 0xFF, 0, 0xFFFFU));
                break;
        }
    }
}

// not used
// void gpio_set_gpio_value(uint8_t pin, uint16_t state)
// {
//     // bool state = dispatch_get_event_state(eventid);
//     for(uint8_t i = 0; i < HASP_NUM_GPIO_CONFIG; i++) {
//         if(gpioConfig[i].pin == pin) {
//             gpio_set_value(gpioConfig[i], state);
//             return;
//         }
//     }
// }

bool gpioIsSystemPin(uint8_t gpio)
{
    if((gpio >= NUM_DIGITAL_PINS) // invalid pins

// Use individual checks instead of switch statement, as some case labels could be duplicated
#ifdef TOUCH_CS
       || (gpio == TOUCH_CS)
#endif
#ifdef TFT_MOSI
       || (gpio == TFT_MOSI)
#endif
#ifdef TFT_MISO
       || (gpio == TFT_MISO)
#endif
#ifdef TFT_SCLK
       || (gpio == TFT_SCLK)
#endif
#ifdef TFT_CS
       || (gpio == TFT_CS)
#endif
#ifdef TFT_DC
       || (gpio == TFT_DC)
#endif
#ifdef TFT_BL
       || (gpio == TFT_BL)
#endif
#ifdef TFT_RST
       || (gpio == TFT_RST)
#endif
#ifdef TFT_WR
       || (gpio == TFT_WR)
#endif
#ifdef TFT_RD
       || (gpio == TFT_RD)
#endif
#ifdef TFT_D0
       || (gpio == TFT_D0)
#endif
#ifdef TFT_D1
       || (gpio == TFT_D1)
#endif
#ifdef TFT_D2
       || (gpio == TFT_D2)
#endif
#ifdef TFT_D3
       || (gpio == TFT_D3)
#endif
#ifdef TFT_D4
       || (gpio == TFT_D4)
#endif
#ifdef TFT_D5
       || (gpio == TFT_D5)
#endif
#ifdef TFT_D6
       || (gpio == TFT_D6)
#endif
#ifdef TFT_D7
       || (gpio == TFT_D7)
#endif
#ifdef TFT_D8
       || (gpio == TFT_D8)
#endif
#ifdef TFT_D9
       || (gpio == TFT_D9)
#endif
#ifdef TFT_D10
       || (gpio == TFT_D10)
#endif
#ifdef TFT_D11
       || (gpio == TFT_D11)
#endif
#ifdef TFT_D12
       || (gpio == TFT_D12)
#endif
#ifdef TFT_D13
       || (gpio == TFT_D13)
#endif
#ifdef TFT_D14
       || (gpio == TFT_D14)
#endif
#ifdef TFT_D15
       || (gpio == TFT_D15)
#endif
    ) {
        return true;
    } // if tft_espi pins

    // To-do:
    // Backlight GPIO
    // Network GPIOs
    // Serial GPIOs
    // Tasmota Client GPIOs

#ifdef ARDUINO_ARCH_ESP32
    if((gpio >= 6) && (gpio <= 11)) return true;  // integrated SPI flash
    if((gpio == 37) || (gpio == 38)) return true; // unavailable
    if(psramFound()) {
        if((gpio == 16) || (gpio == 17)) return true; // PSRAM
    }
#endif

#ifdef ARDUINO_ARCH_ESP8266
    if((gpio >= 6) && (gpio <= 11)) return true; // integrated SPI flash
#ifndef TFT_SPI_OVERLAP
    if((gpio >= 12) && (gpio <= 14)) return true; // HSPI
#endif
#endif

    return false;
}

bool gpioInUse(uint8_t gpio)
{
    for(uint8_t i = 0; i < HASP_NUM_GPIO_CONFIG; i++) {
        if((gpioConfig[i].pin == gpio) && gpioConfigInUse(i)) {
            return true; // pin matches and is in use
        }
    }

    return false;
}

bool gpioSavePinConfig(uint8_t config_num, uint8_t pin, uint8_t type, uint8_t group, uint8_t pinfunc)
{
    // TODO: Input validation

    // ESP8266: Only Pullups except on gpio16

    // ESP32: Pullup or Pulldown except on 34-39

    if(config_num < HASP_NUM_GPIO_CONFIG && !gpioIsSystemPin(pin)) {
        gpioConfig[config_num].pin           = pin;
        gpioConfig[config_num].type          = type;
        gpioConfig[config_num].group         = group;
        gpioConfig[config_num].gpio_function = pinfunc;
        LOG_TRACE(TAG_GPIO, F("Saving Pin config #%d pin %d - type %d - group %d - func %d"), config_num, pin, type,
                  group, pinfunc);
        return true;
    }

    return false;
}

bool gpioConfigInUse(uint8_t num)
{
    if(num >= HASP_NUM_GPIO_CONFIG) return false;
    return gpioConfig[num].type != HASP_GPIO_FREE;
}

int8_t gpioGetFreeConfigId()
{
    uint8_t id = 0;
    while(id < HASP_NUM_GPIO_CONFIG) {
        if(!gpioConfigInUse(id)) return id;
        id++;
    }
    return -1;
}

hasp_gpio_config_t gpioGetPinConfig(uint8_t num)
{
    return gpioConfig[num];
}

////////////////////////////////////////////////////////////////////////////////////////////////////
#if HASP_USE_CONFIG > 0
bool gpioGetConfig(const JsonObject& settings)
{
    bool changed = false;

    /* Check Gpio array has changed */
    JsonArray array = settings[FPSTR(FP_GPIO_CONFIG)].as<JsonArray>();
    uint8_t i       = 0;
    for(JsonVariant v : array) {
        if(i < HASP_NUM_GPIO_CONFIG) {
            uint32_t cur_val = gpioConfig[i].pin | (gpioConfig[i].group << 8) | (gpioConfig[i].type << 16) |
                               (gpioConfig[i].gpio_function << 24);
            LOG_INFO(TAG_GPIO, F("GPIO CONF: %d: %d <=> %d"), i, cur_val, v.as<uint32_t>());

            if(cur_val != v.as<uint32_t>()) changed = true;
            v.set(cur_val);
        } else {
            changed = true;
        }
        i++;
    }

    /* Build new Gpio array if the count is not correct */
    if(i != HASP_NUM_GPIO_CONFIG) {
        array = settings[FPSTR(FP_GPIO_CONFIG)].to<JsonArray>(); // Clear JsonArray
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
bool gpioSetConfig(const JsonObject& settings)
{
    configOutput(settings);
    bool changed = false;

    if(!settings[FPSTR(FP_GPIO_CONFIG)].isNull()) {
        bool status = false;
        int i       = 0;

        JsonArray array = settings[FPSTR(FP_GPIO_CONFIG)].as<JsonArray>();
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
#endif // HASP_USE_CONFIG
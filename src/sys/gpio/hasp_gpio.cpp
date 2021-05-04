/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "lv_conf.h" // For timing defines

#include "hasplib.h"
#include "hasp_gpio.h"
#include "hasp_config.h"

#ifdef ARDUINO_ARCH_ESP8266
#define INPUT_PULLDOWN INPUT
#endif

#ifdef ARDUINO
#include "AceButton.h"
using namespace ace_button;
static AceButton* button[HASP_NUM_INPUTS];
#else
#define HIGH 1
#define LOW 0
#define NUM_DIGITAL_PINS 40
#define digitalWrite(x, y)
#define analogWrite(x, y)
#endif

#define SCALE_8BIT_TO_12BIT(x) x << 4 | x >> 4
#define SCALE_8BIT_TO_10BIT(x) x << 2 | x >> 6

uint8_t gpioUsedInputCount = 0;

// An array of button pins, led pins, and the led states. Cannot be const
// because ledState is mutable.
hasp_gpio_config_t gpioConfig[HASP_NUM_GPIO_CONFIG] = {
    //    {2, 8, INPUT, LOW}, {3, 9, OUTPUT, LOW}, {4, 10, INPUT, HIGH}, {5, 11, OUTPUT, LOW}, {6, 12, INPUT, LOW},
};

#if defined(ARDUINO_ARCH_ESP32)
#include "driver/uart.h"
#include <driver/dac.h>

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

void gpio_log_serial_dimmer(const char* command)
{
    char buffer[32];
    snprintf_P(buffer, sizeof(buffer), PSTR("Dimmer => %02x %02x %02x %02x"), command[0], command[1], command[2],
               command[3]);
    LOG_VERBOSE(TAG_GPIO, buffer);
}

#ifdef ARDUINO
static void gpio_event_handler(AceButton* button, uint8_t eventType, uint8_t buttonState)
{
    uint8_t btnid = button->getId();
    uint8_t eventid;
    bool state = false;
    switch(eventType) {
        case AceButton::kEventPressed:
            if(gpioConfig[btnid].type == HASP_GPIO_SWITCH) {
                eventid = HASP_EVENT_ON;
            } else {
                eventid = HASP_EVENT_DOWN;
            }
            state = true;
            break;
        case 2: // AceButton::kEventClicked:
            eventid = HASP_EVENT_UP;
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
            if(gpioConfig[btnid].type == HASP_GPIO_SWITCH) {
                eventid = HASP_EVENT_OFF;
            } else {
                eventid = HASP_EVENT_RELEASE;
            }
            break;
        default:
            eventid = HASP_EVENT_LOST;
    }

    event_gpio_input(gpioConfig[btnid].pin, gpioConfig[btnid].group, eventid);
    if(eventid != HASP_EVENT_LONG) // do not repeat DOWN + LONG
        dispatch_normalized_group_values(gpioConfig[btnid].group, NULL, state, HASP_EVENT_OFF, HASP_EVENT_ON);
}

/* ********************************* GPIO Setup *************************************** */

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

void gpio_setup_pin(hasp_gpio_config_t* gpio)
{
    uint8_t input_mode;
    switch(gpio->gpio_function) {
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

    if(gpioIsSystemPin(gpio->pin)) {
        LOG_WARNING(TAG_GPIO, F("Invalid pin %d"), gpio->pin);
        return;
    }

    gpio->max = 255;

    switch(gpio->type) {
        case HASP_GPIO_SWITCH:
            gpioAddSwitch(gpio->pin, input_mode, HIGH, gpioUsedInputCount);
            pinMode(gpio->pin, INPUT_PULLUP);
            gpio->max = 0;
            break;

        case HASP_GPIO_BUTTON:
            gpioAddButton(gpio->pin, input_mode, HIGH, gpioUsedInputCount);
            pinMode(gpio->pin, INPUT_PULLUP);
            gpio->max = 0;
            break;

        case HASP_GPIO_RELAY:
            pinMode(gpio->pin, OUTPUT);
            gpio->max = 1; // on-off
            break;

        case HASP_GPIO_PWM:
            gpio->max = 4095;
        case HASP_GPIO_ALL_LEDS:
            // case HASP_GPIO_BACKLIGHT:
            pinMode(gpio->pin, OUTPUT);
#if defined(ARDUINO_ARCH_ESP32)
            // configure LED PWM functionalitites
            ledcSetup(gpio->group, 20000, 12);
            // attach the channel to the GPIO to be controlled
            ledcAttachPin(gpio->pin, gpio->group);
#endif
            break;

        case HASP_GPIO_DAC:
#if defined(ARDUINO_ARCH_ESP32)
            gpio_num_t pin;
            if(dac_pad_get_io_num(DAC_CHANNEL_1, &pin) == ESP_OK)
                if(gpio->pin == pin) dac_output_enable(DAC_CHANNEL_1);
            if(dac_pad_get_io_num(DAC_CHANNEL_2, &pin) == ESP_OK)
                if(gpio->pin == pin) dac_output_enable(DAC_CHANNEL_2);
#endif
            break;

        case HASP_GPIO_SERIAL_DIMMER: {
            const char command[9] = "\xEF\x01\x4D\xA3"; // Start Lanbon Dimmer
#if defined(ARDUINO_ARCH_ESP32)
            Serial1.begin(115200UL, SERIAL_8N1, UART_PIN_NO_CHANGE, gpio->pin, true, 2000);
            Serial1.flush();
            delay(20);
            Serial1.print("  ");
            delay(20);
            Serial1.write((const uint8_t*)command, 8);
#endif
            gpio_log_serial_dimmer(command);
            break;
        }

        case HASP_GPIO_FREE:
            return;

        default:
            LOG_WARNING(TAG_GPIO, F("Invalid config -> pin %d - type: %d"), gpio->pin, gpio->type);
    }
    LOG_VERBOSE(TAG_GPIO, F(D_BULLET "Configured pin %d"), gpio->pin);
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
    LOG_INFO(TAG_GPIO, F(D_SERVICE_STARTING));

    aceButtonSetup();

    for(uint8_t i = 0; i < HASP_NUM_GPIO_CONFIG; i++) {
        gpio_setup_pin(&gpioConfig[i]);
    }

    LOG_INFO(TAG_GPIO, F(D_SERVICE_STARTED));
}

void gpioLoop(void)
{
    // Should be called every 4-5ms or faster, for the default debouncing time of ~20ms.
    for(uint8_t i = 0; i < gpioUsedInputCount; i++) {
        if(button[i]) button[i]->check();
    }
}

#else
void gpioSetup(void)
{
    gpioSavePinConfig(0, 3, HASP_GPIO_RELAY, 0, -1);
    gpioSavePinConfig(1, 4, HASP_GPIO_RELAY, 0, -1);
    gpioSavePinConfig(2, 13, HASP_GPIO_LED, 0, -1);
    gpioSavePinConfig(3, 14, HASP_GPIO_LEDAC, 0, -1);
}
void gpioLoop(void)
{}
#endif // ARDUINO

/* ********************************* State Setters *************************************** */

bool gpio_get_value(uint8_t pin, uint16_t& val)
{
    for(uint8_t i = 0; i < HASP_NUM_GPIO_CONFIG; i++) {
        if(gpioConfig[i].pin == pin && gpioConfigInUse(i)) {
            val = gpioConfig[i].val;
            return true;
        }
    }
    return false;
}

static inline int32_t gpio_limit(int32_t val, int32_t min, int32_t max)
{
    if(val >= max) return max;
    if(val <= min) return min;
    return val;
}

// val is assumed to be 12 bits
static inline bool gpio_set_analog_value(hasp_gpio_config_t* gpio)
{
    uint16_t val = 0;
#if defined(ARDUINO_ARCH_ESP32)

    if(gpio->max == 255)
        val = SCALE_8BIT_TO_12BIT(gpio->val);
    else if(gpio->max == 4095)
        val = gpio->val;

    if(gpio->inverted) val = 4095 - val;

    ledcWrite(gpio->group, val); // 12 bits
    return true;                 // sent

#elif defined(ARDUINO_ARCH_ESP8266)

    if(gpio->max == 255)
        val = SCALE_8BIT_TO_10BIT(gpio->val);
    else if(gpio->max == 4095)
        val = gpio->val >> 2;

    if(gpio->inverted) val = 1023 - val;

    analogWrite(gpio->pin, val); // 10 bits
    return true;                 // sent

#else
    return false; // not implemented
#endif
}

static inline bool gpio_set_serial_dimmer(hasp_gpio_config_t* gpio)
{
    char command[5] = "\xEF\x02\x00\xED";
    command[2]      = (uint8_t)map(gpio->val, 0, 255, 0, 100);
    command[3] ^= command[2];

#if defined(ARDUINO_ARCH_ESP32)
    Serial1.write((const uint8_t*)command, 4);
    gpio_log_serial_dimmer(command);
    return true; // sent
#else
    gpio_log_serial_dimmer(command);
    return false; // not sent
#endif
}

static inline bool gpio_set_dac_value(hasp_gpio_config_t* gpio)
{
#ifdef ARDUINO_ARCH_ESP32
    gpio_num_t pin;
    if(dac_pad_get_io_num(DAC_CHANNEL_1, &pin) == ESP_OK && gpio->pin == pin)
        dac_output_voltage(DAC_CHANNEL_1, gpio->val);
    else if(dac_pad_get_io_num(DAC_CHANNEL_2, &pin) == ESP_OK && gpio->pin == pin)
        dac_output_voltage(DAC_CHANNEL_2, gpio->val);
    else
        return false; // not found
    return true;      // found
#else
    return false; // not implemented
#endif
}

bool gpio_get_pin_config(uint8_t pin, hasp_gpio_config_t** gpio)
{
    for(uint8_t i = 0; i < HASP_NUM_GPIO_CONFIG; i++) {
        if(gpioConfig[i].pin == pin) {
            *gpio = &gpioConfig[i];
            return true;
        }
    }
    return false;
}

// Update the actual value of one pin
// The value must be normalized first
static bool gpio_set_output_value(hasp_gpio_config_t* gpio, uint16_t val)
{
    gpio->val = gpio_limit(val, 0, gpio->max);

    switch(gpio->type) {
        case HASP_GPIO_RELAY:
            digitalWrite(gpio->pin, gpio->inverted ? !gpio->val : gpio->val);
            return true;

        case HASP_GPIO_ALL_LEDS:
        case HASP_GPIO_PWM:
            return gpio_set_analog_value(gpio);

        case HASP_GPIO_DAC:
            return gpio_set_dac_value(gpio);

        case HASP_GPIO_SERIAL_DIMMER:
            return gpio_set_serial_dimmer(gpio);

        default:
            LOG_WARNING(TAG_GPIO, F(D_BULLET "Pin %d is not a valid output"), gpio->pin);
            return false; // not a valid output
    }
}

// Update the normalized value of one pin
void gpio_set_normalized_value(hasp_gpio_config_t* gpio, int32_t val, int32_t min, int32_t max)
{
    if(min != 0 || max != gpio->max) {
        if(min == max) {
            LOG_ERROR(TAG_GPIO, F("Invalid value range"));
            return;
        }

        switch(gpio->type) {
            case HASP_GPIO_RELAY:
                val = val > min ? HIGH : LOW;
                break;

            case HASP_GPIO_ALL_LEDS:
            case HASP_GPIO_DAC:
            case HASP_GPIO_PWM:
            case HASP_GPIO_SERIAL_DIMMER:
                val = map(val, min, max, 0, gpio->max);
                break;

            default:
                return;
        }
    }
    gpio_set_output_value(gpio, val); // normalized
}

/* void gpio_set_normalized_group_values(uint8_t groupid, int16_t val, int16_t min, int16_t max)
{
    if(min == max) {
        LOG_ERROR(TAG_GPIO, F("Invalid value range"));
        return;
    }

    // bool state = Parser::get_event_state(eventid);
    for(uint8_t i = 0; i < HASP_NUM_GPIO_CONFIG; i++) {
        if(gpioConfig[i].group == groupid) {
            gpio_set_normalized_value(&gpioConfig[i], val, min, max);
        }
    }
} */

// Dispatch all group member values
void gpio_output_group_values(uint8_t group)
{
    for(uint8_t k = 0; k < HASP_NUM_GPIO_CONFIG; k++) {
        hasp_gpio_config_t* gpio = &gpioConfig[k];
        if(gpio->group == group && gpio->type != HASP_GPIO_BUTTON &&
           gpio->type != HASP_GPIO_SWITCH && // group members that are outputs
           gpioConfigInUse(k))
            dispatch_output_pin_value(gpioConfig[k].pin, gpioConfig[k].val);
    }
}

// Update the normalized value of all group members
void gpio_set_normalized_group_values(uint8_t group, int32_t val, int32_t min, int32_t max)
{
    // Set all pins first, minimizes delays
    for(uint8_t k = 0; k < HASP_NUM_GPIO_CONFIG; k++) {
        hasp_gpio_config_t* gpio = &gpioConfig[k];
        if(gpio->group == group && // group members that are outputs
           gpioConfigInUse(k))
            gpio_set_normalized_value(gpio, val, min, max);
    }

    gpio_output_group_values(group);
    object_set_normalized_group_values(group, NULL, val, min, max); // Update onsreen objects
}

// Update the value of an output pin and its group members
bool gpio_set_pin_value(uint8_t pin, int32_t val)
{
    hasp_gpio_config_t* gpio = NULL;

    if(!gpio_get_pin_config(pin, &gpio) || !gpio || gpio->type == HASP_GPIO_FREE) {
        LOG_WARNING(TAG_GPIO, F(D_BULLET "Pin %d is not configured"), pin);
        return false;

    } else if(gpio->type == HASP_GPIO_BUTTON || gpio->type == HASP_GPIO_SWITCH) {
        LOG_WARNING(TAG_GPIO, F(D_BULLET "Pin %d is an input"), pin);
        if(gpio->group) gpio_output_group_values(gpio->group);
        return false;
    }

    if(gpio->group) {
        gpio_set_normalized_group_values(gpio->group, val, 0, gpio->max); // Set all pins in the group
        LOG_VERBOSE(TAG_GPIO, F("Group %d - Pin %d = %d"), gpio->group, gpio->pin, gpio->val);

    } else {
        gpio_set_output_value(gpio, val); // update this gpio value only
        dispatch_output_pin_value(gpio->pin, gpio->val);
        LOG_VERBOSE(TAG_GPIO, F("No Group - Pin %d = %d"), gpio->pin, gpio->val);
    }

    return true; // pin found and set
}

// Updates the RGB pins directly, rgb are already normalized values
void gpio_set_moodlight(uint8_t r, uint8_t g, uint8_t b)
{
    // RGBXX https://stackoverflow.com/questions/39949331/how-to-calculate-rgbaw-amber-white-from-rgb-for-leds
    for(uint8_t i = 0; i < HASP_NUM_GPIO_CONFIG; i++) {
        switch(gpioConfig[i].type) {
            case HASP_GPIO_LED_R:
                gpio_set_output_value(&gpioConfig[i], r);
                break;
            case HASP_GPIO_LED_G:
                gpio_set_output_value(&gpioConfig[i], g);
                break;
            case HASP_GPIO_LED_B:
                gpio_set_output_value(&gpioConfig[i], b);
                break;
        }
    }
}

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

void gpio_discovery(JsonArray& relay, JsonArray& led)
{
    for(uint8_t i = 0; i < HASP_NUM_GPIO_CONFIG; i++) {
        switch(gpioConfig[i].type) {
            case HASP_GPIO_RELAY:
                relay.add(gpioConfig[i].pin);
                break;

            case HASP_GPIO_DAC:
            case HASP_GPIO_LED: // Don't include the moodlight
            case HASP_GPIO_SERIAL_DIMMER:
                led.add(gpioConfig[i].pin);
                break;

                // pwm.add(gpioConfig[i].pin);
                break;

            case HASP_GPIO_FREE:
            default:
                break;
        }
    }
};

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
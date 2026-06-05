/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "lanbonl8.h"

#if defined(LANBONL8)

#include <Arduino.h>
#include "dev/esp32/esp32.h"

#include "driver/pcnt.h" // Pulse count driver
#if ESP_ARDUINO_VERSION_MAJOR >= 2
#include "hal/pcnt_hal.h"
#include "hal/gpio_hal.h"
#include "soc/pcnt_periph.h"
#include "esp_rom_gpio.h"
#endif

#include "driver/adc.h"
#include "esp_adc_cal.h"

#include "hasp_conf.h"
#include "hasp_debug.h"

#define BACKLIGHT_CHANNEL 15

// https://esp32.com/viewtopic.php?t=14660
#define PCNT_FREQ_UNIT PCNT_UNIT_0 // Pulse Count Unit 0
#define PCNT_H_LIM_VAL 10000       // upper limit of counting  max. 32767, write +1 to overflow counter, when reached
#define PCNT_L_LIM_VAL -10000      // lower limit of counting  max. 32767, write +1 to overflow counter, when reached
#define PCNT_INPUT_SIG_IO 35       // Pulse Input GPIO
#define PCNT_INPUT_CTRL_IO 36      // Pulse Control GPIO
#define PCNT_FILTER_VAL 300        // filter (damping, inertia) value for avoiding glitches in the count, max. 1023
#define MEASURED_WATTS 1580
#define MEASURED_PULSES_PER_SECOND 281 // Pulses / Per second

int16_t PulseCounter = 0; // pulse counter, max. value is 32535
int OverflowCounter  = 0; // pulse counter overflow counter
uint32_t pulseOffset = 0; // total historical pulses, used to calculate total energy consumption across power cycles
uint32_t totalPulses;

pcnt_isr_handle_t user_isr_handle = NULL; // interrupt handler - not used
hw_timer_t* timer                 = NULL; // Instancia do timer

#define REF_VOLTAGE 1100
esp_adc_cal_characteristics_t* adc_chars =
    new esp_adc_cal_characteristics_t; // adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));

int16_t watt_10 = 0;
//int16_t kwh_10 = 0;
float kwh = 0;

namespace dev {

static void energy_save()
{
    Preferences preferences;
    if (nvs_user_begin(preferences, "energy", false)){
        preferences.putUInt("pulses", totalPulses + pulseOffset);
        preferences.end();
    }
}

static void energy_restore()
{
    Preferences preferences;
    if (nvs_user_begin(preferences, "energy", true)){
        pulseOffset = preferences.getUInt("pulses", 0);
        preferences.end();
    }
}

static void moodlight_save(moodlight_t& ml)
{
    Preferences preferences;
    if(nvs_user_begin(preferences, "moodlight", false)) {
        preferences.putUChar("brightness", ml.brightness);
        preferences.putUChar("power", ml.power);
        preferences.putBytes("rgbww", ml.rgbww, sizeof(ml.rgbww));
        preferences.end();
    }
}

static void moodlight_restore(moodlight_t& ml)
{
    Preferences preferences;
    if(nvs_user_begin(preferences, "moodlight", true)) {
        ml.brightness = preferences.getUChar("brightness", 255);
        ml.power      = preferences.getUChar("power", 0);
        preferences.getBytes("rgbww", ml.rgbww, sizeof(ml.rgbww));
        preferences.end();
    }
}

static void backlight_save(uint8_t level, bool power)
{
    Preferences preferences;
    if(nvs_user_begin(preferences, "backlight", false)) {
        preferences.putUChar("level", level);
        preferences.putBool("power", power);
        preferences.end();
    }
}

static void backlight_restore(uint8_t& level, bool& power)
{
    Preferences preferences;
    if(nvs_user_begin(preferences, "backlight", true)) {
        level = preferences.getUChar("level", 255);
        power = preferences.getBool("power", true);
        preferences.end();
    }
}

void LanbonL8::set_backlight_level(uint8_t level)
{
    Esp32Device::set_backlight_level(level);
    backlight_save(level, get_backlight_power());
}

void LanbonL8::set_backlight_power(bool power)
{
    Esp32Device::set_backlight_power(power);
    backlight_save(get_backlight_level(), power);
}

void LanbonL8::energy_reset()
{
    pulseOffset = 0;
    totalPulses = 0;
    OverflowCounter = 0;
    pcnt_counter_clear(PCNT_FREQ_UNIT);
    Preferences preferences;
    if(nvs_user_begin(preferences, "energy", false)) {
        preferences.putUInt("pulses", 0);
        preferences.end();
    }
}

static void check_efuse(void)
{

    // Check TP is burned into eFuse
    if(esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        printf("eFuse Two Point: Supported\n");
    } else {
        printf("eFuse Two Point: NOT supported\n");
    }

    // Check Vref is burned into eFuse
    if(esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
        printf("eFuse Vref: Supported\n");
    } else {
        printf("eFuse Vref: NOT supported\n");
    }
}

static void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if(val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        Serial.print("Characterized using Two Point Value\n");
    } else if(val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        Serial.print("Characterized using eFuse Vref\n");
    } else {
        Serial.print("Characterized using Default Vref\n");
    }
}

//------------------------------------------------------------------------------------
void IRAM_ATTR energy_pulse_counter_overflow(void* arg)
{                                           // Interrupt for overflow of pulse counter
    OverflowCounter  = OverflowCounter + 1; // increase overflow counter
    PCNT.int_clr.val = BIT(PCNT_FREQ_UNIT); // clean overflow flag
    pcnt_counter_clear(PCNT_FREQ_UNIT);     // zero and reset of pulse counter unit
}

//------------------------------------------------------------
void energy_pulse_counter_init()
{
    pcnt_config_t pcntFreqConfig  = {};                 // initialise pulse counter
    pcntFreqConfig.pulse_gpio_num = PCNT_INPUT_SIG_IO;  // pin assignment for pulse counter
    pcntFreqConfig.ctrl_gpio_num  = PCNT_INPUT_CTRL_IO; // pin assignment for  control
    pcntFreqConfig.channel        = PCNT_CHANNEL_0;     // select channel 0 of pulse counter unit 0
    pcntFreqConfig.unit           = PCNT_FREQ_UNIT;     // select ESP32 pulse counter unit 0
    pcntFreqConfig.pos_mode   = PCNT_COUNT_INC; // count rising edges (=change from low to high logical level) as pulses
    pcntFreqConfig.neg_mode   = PCNT_COUNT_DIS; // Conta na subida do pulso
    pcntFreqConfig.lctrl_mode = PCNT_MODE_REVERSE;
    pcntFreqConfig.hctrl_mode = PCNT_MODE_KEEP,
    pcntFreqConfig.counter_h_lim = PCNT_H_LIM_VAL; // set upper limit of counting
    pcntFreqConfig.counter_l_lim = PCNT_L_LIM_VAL; // set lower limit of counting

    pcnt_unit_config(&pcntFreqConfig); // configure rigisters of the pulse counter

    pcnt_counter_pause(PCNT_FREQ_UNIT); // pause puls counter unit
    pcnt_counter_clear(PCNT_FREQ_UNIT); // zero and reset of pulse counter unit

    pcnt_event_enable(PCNT_FREQ_UNIT, PCNT_EVT_H_LIM); // enable event for interrupt on reaching upper limit of counting
    pcnt_isr_register(energy_pulse_counter_overflow, NULL, 0,
                      &user_isr_handle); // configure register overflow interrupt handler
    pcnt_intr_enable(PCNT_FREQ_UNIT);    // enable overflow interrupt

    pcnt_set_filter_value(PCNT_FREQ_UNIT, PCNT_FILTER_VAL); // set damping, inertia
    pcnt_filter_enable(PCNT_FREQ_UNIT);                     // enable counter glitch filter (damping)

    pcnt_counter_resume(PCNT_FREQ_UNIT); // resume counting on pulse counter unit
}

void LanbonL8::init()
{
    energy_pulse_counter_init();
    energy_restore();

    // Restore backlight state
    uint8_t level = 255;
    bool power    = true;
    backlight_restore(level, power);
    Esp32Device::set_backlight_level(level);
    Esp32Device::set_backlight_power(power);

    // Check if Two Point or Vref are burned into eFuse
    check_efuse();

    // Characterize ADC
    adc1_config_width(ADC_WIDTH_12Bit);
    adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN_0db);
    esp_adc_cal_value_t val_type =
        esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_0db, ADC_WIDTH_12Bit, REF_VOLTAGE, adc_chars);
    print_char_val_type(val_type);
}

void LanbonL8::loop_5s()
{                                                          // function for reading pulse counter (for timer)
    pcnt_get_counter_value(PCNT_FREQ_UNIT, &PulseCounter); // get pulse counter value - maximum value is 16 bits
    uint32_t newPulses = OverflowCounter * 10000 + PulseCounter;
    uint32_t delta     = newPulses - totalPulses;
    totalPulses        = newPulses;
    watt_10            = DEC / 5 * delta * MEASURED_WATTS / MEASURED_PULSES_PER_SECOND;
    //kwh_10             = DEC * totalPulses * MEASURED_WATTS / MEASURED_PULSES_PER_SECOND / 3600 / 1000;
    kwh                = (float)((totalPulses + pulseOffset) * MEASURED_WATTS / MEASURED_PULSES_PER_SECOND / 3600.0f / 1000.0f);

    if (++_save_counter >= 720){ //trigger hourly
        _save_counter = 0;
        energy_save();
    }
    // LOG_VERBOSE(TAG_DEV, F("Pulse Counter %d.%d W / %d / %d.%d kWh"), watt_10 / DEC, watt_10 % DEC, totalPulses,
    //             kwh_10 / DEC, kwh_10 % DEC);
    // uint32_t temp = (temprature_sens_read() - 32) * 100 / 1.8;
    // LOG_VERBOSE(TAG_DEV, F("Temperature %d C"), temp);
}

void LanbonL8::get_sensors(JsonDocument& doc)
{
    Esp32Device::get_sensors(doc);

    JsonObject sensor = doc.createNestedObject(F("Energy"));

    //  int16_t kwh_10     = DEC * totalPulses * MEASURED_WATTS / MEASURED_PULSES_PER_SECOND / 3600 / 1000;

    /* Pulse counter Stats */
    sensor[F("Power")] = serialized(String(1.0f * watt_10 / DEC, 2));
    sensor[F("Total")] = serialized(String(kwh, 3));
}

//------------------------------------------------------------
void Read_Reset_PCNT()
{                                                          // function for reading pulse counter (for timer)
    pcnt_get_counter_value(PCNT_FREQ_UNIT, &PulseCounter); // get pulse counter value - maximum value is 16 bits

    // resetting counter as if example, delet for application in PiedPiperS
    OverflowCounter = 0;                // set overflow counter to zero
    pcnt_counter_clear(PCNT_FREQ_UNIT); // zero and reset of pulse counter unit
    // conterOK = true;                                         // not in use, copy from example code
    // ########################################
}

} // namespace dev

dev::LanbonL8 haspDevice;

#endif

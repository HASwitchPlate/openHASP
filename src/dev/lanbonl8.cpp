#include "Arduino.h"
#include "lanbonl8.h"

#if defined(LANBONL8)

    #include "driver/adc.h"
    #include "esp_adc_cal.h"

    #define REF_VOLTAGE 1100
esp_adc_cal_characteristics_t * adc_chars =
    new esp_adc_cal_characteristics_t; // adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));

namespace dev {

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

void LanbonL8::pre_setup()
{
    // Check if Two Point or Vref are burned into eFuse
    check_efuse();

    // Characterize ADC
    adc1_config_width(ADC_WIDTH_12Bit);
    adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN_0db);
    esp_adc_cal_value_t val_type =
        esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_0db, ADC_WIDTH_12Bit, REF_VOLTAGE, adc_chars);
    print_char_val_type(val_type);
}

void LanbonL8::post_setup()
{}

void LanbonL8::loop()
{}

void LanbonL8::loop_5s()
{
    double voltage = esp_adc_cal_raw_to_voltage(analogRead(39), adc_chars);
    Serial.print(adc1_get_raw(ADC1_CHANNEL_3));
    Serial.print(" - ");
    Serial.println(voltage);
}
} // namespace dev

dev::LanbonL8 haspDevice;
#endif

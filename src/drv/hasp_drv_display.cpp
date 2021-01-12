#include "hasp_drv_display.h"

#if defined(M5STACK)
    #include "AXP192.h" // Power Mgmt

AXP192 Axp;

void m5stack_init()
{
    AXP192 Axp;
    Wire.begin(TOUCH_SDA, TOUCH_SCL);
    Axp.begin();

    Axp.SetCHGCurrent(AXP192::kCHG_100mA);
    Axp.SetLcdVoltage(2800);

    Axp.SetBusPowerMode(0);
    Axp.SetCHGCurrent(AXP192::kCHG_190mA);

    Axp.SetLDOEnable(3, true);
    //    CoverScrollText("Motor Test", M5.Lcd.color565(SUCCE_COLOR));
    delay(150);
    Axp.SetLDOEnable(3, false);

    Axp.SetLed(1);
    //   CoverScrollText("LED Test", M5.Lcd.color565(SUCCE_COLOR));
    delay(100);
    Axp.SetLed(0);

    // FastLED.addLeds<SK6812, LEDS_PIN>(ledsBuff, LEDS_NUM);
    // for(int i = 0; i < LEDS_NUM; i++) {
    //     ledsBuff[i].setRGB(20, 20, 20);
    // }
    // FastLED.show();

    Axp.SetLDOVoltage(3, 3300);
    Axp.SetLed(1);
}
#endif

void drv_display_init(lv_disp_drv_t * disp_drv, uint8_t rotation, bool invert_display)
{
#if defined(M5STACK)
    m5stack_init(); // Set LCD power first
#endif

    /* TFT init */
#if defined(USE_FSMC)
    fsmc_ili9341_init(rotation, invert_display);
    disp_drv->flush_cb = fsmc_ili9341_flush; // Normal callback when flushing
    // xpt2046_init(rotation);
#else
    tft_espi_init(rotation, invert_display);
    disp_drv->flush_cb = tft_espi_flush; // Normal callback when flushing
#endif
}

/* Callback used for screenshots only: */

/* indirect callback to flush screenshot data to the screen */
void drv_display_flush_cb(lv_disp_drv_t * disp, const lv_area_t * area, lv_color_t * color_p)
{
#if defined(USE_FSMC)
    fsmc_ili9341_flush(disp, area, color_p);
#else
    tft_espi_flush(disp, area, color_p);
#endif
}
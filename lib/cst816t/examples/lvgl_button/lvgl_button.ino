/* lvgl demo on P169H002-CTP display */

#include <SPI.h>
#include <Adafruit_GFX.h>     // Core graphics library
#include <Adafruit_ST7789.h>  // Hardware-specific library for ST7789

#include <Wire.h>
#include "cst816t.h"  // capacitive touch

/* Note: in lv_conf.h: set lv_tick to Arduino millis() */

#include <lvgl.h>

// display
#define TFT_X 240
#define TFT_Y 280

#define TFT_CS PB1
#define TFT_RST PA4
#define TFT_DC PB0
#define TFT_MOSI PB15
#define TFT_SCLK PB13
#define TFT_LED PA8

// touch
#define TP_SDA PB11
#define TP_SCL PB10
#define TP_RST PA15
#define TP_IRQ PB3

TwoWire Wire2(TP_SDA, TP_SCL);
cst816t touchpad(Wire2, TP_RST, TP_IRQ);

SPIClass SPI_1(PB15, PB14, PB13);
Adafruit_ST7789 tft = Adafruit_ST7789(&SPI_1, TFT_CS, TFT_DC, TFT_RST);

static lv_disp_draw_buf_t disp_buf;
static lv_color_t buf_1[TFT_X * 10];
static lv_color_t buf_2[TFT_X * 10];
static lv_disp_drv_t disp_drv;
static lv_indev_drv_t indev_drv;

/* lvgl display output */
/* Using Adafruit display driver. Depending upon architecture, other drivers may be faster. */
void disp_flush_cb(lv_disp_drv_t* disp_drv, const lv_area_t* area, lv_color_t* color_p) {
  uint32_t w = 1 + area->x2 - area->x1;
  uint32_t h = 1 + area->y2 - area->y1;
  uint32_t len = w * h;
  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.writePixels(&color_p->full, len);
  tft.endWrite();
  lv_disp_flush_ready(disp_drv);
}

/* lvgl touchpad input */
void touchpad_input_read(lv_indev_drv_t* drv, lv_indev_data_t* data) {
  static uint32_t tp_x = 0;
  static uint32_t tp_y = 0;
  static uint32_t tp_fingers = 0;
  if (touchpad.available()) {
    tp_x = touchpad.x;
    tp_y = touchpad.y;
    tp_fingers = touchpad.finger_num;
  }
  data->point.x = tp_x;
  data->point.y = tp_y;
  if (tp_fingers != 0) data->state = LV_INDEV_STATE_PRESSED;
  else data->state = LV_INDEV_STATE_RELEASED;
}

/* lvgl button callback */
void btn_event_cb(lv_event_t* e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t* btn = lv_event_get_target(e);
  if (code == LV_EVENT_CLICKED) {
    static uint8_t cnt = 0;
    cnt++;
    /*Get the first child of the button which is the label and change its text*/
    lv_obj_t* label = lv_obj_get_child(btn, 0);
    lv_label_set_text_fmt(label, "Button %d", cnt);
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("boot");

  analogWriteResolution(8);
  analogWrite(TFT_LED, 127);  // display backlight at 50%

  tft.init(240, 280);  // Init ST7789 280x240
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setRotation(2);

  touchpad.begin(mode_change);

  /* display */
  lv_init();
  lv_disp_draw_buf_init(&disp_buf, buf_1, buf_2, TFT_X * 10);
  lv_disp_drv_init(&disp_drv);
  disp_drv.draw_buf = &disp_buf;
  disp_drv.flush_cb = disp_flush_cb;
  disp_drv.hor_res = TFT_X;
  disp_drv.ver_res = TFT_Y;
  lv_disp_t* disp;
  disp = lv_disp_drv_register(&disp_drv);

  /* touchpad */
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = touchpad_input_read;
  lv_indev_t* my_indev = lv_indev_drv_register(&indev_drv);

  /* button */
  lv_obj_t* btn = lv_btn_create(lv_scr_act());
  lv_obj_center(btn);
  lv_obj_set_size(btn, 120, 50);
  lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_t* label = lv_label_create(btn);
  lv_label_set_text(label, "Button");
  lv_obj_center(label);
}

void loop() {
  lv_timer_handler();
}

include $(LVGL_DIR)/lv_drivers/display/display.mk
include $(LVGL_DIR)/lv_drivers/indev/indev.mk


CSRCS += win_drv.c

DEPPATH += --dep-path $(LVGL_DIR)/lv_drivers
VPATH += :$(LVGL_DIR)/lv_drivers

CFLAGS += "-I$(LVGL_DIR)/lv_drivers"

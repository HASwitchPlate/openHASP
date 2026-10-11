# CrowPanel Advance 5.0 (DIS02050A, PCB V1.3)

Use the `elecrow-s3-8048c050-advance-v13_16MB` environment for the
DIS02050A V1.3 board with 16 MB flash and 8 MB PSRAM. This is a separate
profile from the older WZ8048C050 and WZ8048C070 boards; their RGB and touch
pin assignments must not be reused for this display. The profile is not
intended for the 7-inch Advance board.

Include `user_setups/esp32s3/crowpanel-hmi.ini` in the PlatformIO
configuration's `extra_configs`, then select this environment by name.

The profile configures the 800 × 480 RGB panel and GT911 touch controller
using Elecrow's [V1.2/V1.3 LovyanGFX example](https://github.com/Elecrow-RD/CrowPanel-Advance-5-HMI-ESP32-S3-AI-Powered-IPS-Touch-Screen-800x480/blob/19b0f58cdb046493238f1dbf2e04215cf8d6bdec/example/V1.2_and_V1.3/Arduino/lesson-03/BigInch_LVGL/LovyanGFX_Driver.h).
GPIO15/16 carry the shared I²C bus. The board's controller at address `0x30`
handles backlight commands; the GT911 is at `0x5D`. Backlight values are
inverted: `0` is brightest and `245` is off. Values above `245` are reserved
for controller commands. Startup performs bounded touch activation using
GPIO1 if the GT911 does not initially respond. If the controller at `0x30`
is missing, the board-specific code does not pulse GPIO1 or send backlight
commands. These behaviors follow Elecrow's [V1.2/V1.3 startup example](https://github.com/Elecrow-RD/CrowPanel-Advance-5-HMI-ESP32-S3-AI-Powered-IPS-Touch-Screen-800x480/blob/19b0f58cdb046493238f1dbf2e04215cf8d6bdec/example/V1.2_and_V1.3/Arduino/lesson-03/BigInch_LVGL/BigInch_LVGL.ino).

Before using the profile on hardware, confirm the PCB marking is V1.3 and
the module is the N16R8 variant. After flashing, verify that the image,
touch coordinates, brightness and off/on commands work. A successful build
alone cannot validate the display wiring or board-controller behavior.

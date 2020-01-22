# HASP - Open Hardware edition
This is a re-implementation of the popular HASwitchPlate sketch created by aderusha.
The [original HASwitchPlate][1] project uses a Wemos D1 mini and requires a Nextion/TJC HMI display.
This fork removes the Nextion/TJC requirement by using the Littlev Graphics Library on the MCU to drive a commodity display.

This version also adds ESP32 support to take advantage of the additional hardware capabilities.

[1]: https://github.com/aderusha/HASwitchPlate

## Features

| Feature            | ESP8266 | ESP32
|--------------------|---------|---------
| SPI display        | <ul><li>- [x] yes</li> | <ul><li>- [x] yes</li>
| Parallel display   | <ul><li>- [ ] no</li> | *tbd*
| PWM Screen dimming | <ul><li>- [x] yes</li> | <ul><li>- [x] yes</li>
| Maximum Page Count | 4       | 12
| Dynamic Objects    | <ul><li>- [x] yes</li> | <ul><li>- [x] yes</li>
| Lvgl Theme Support | basic themes | all themes
| Custom .zi V5 font | <ul><li>- [x] yes</li> | <ul><li>- [x] yes</li>
| FontAwesome Icons  | <ul><li>- [x] 1200+</li> | <ul><li>- [x] 1200+</li>
| PNG images         | <ul><li>- [ ] no</li> | *tbd*

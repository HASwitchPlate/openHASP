# HASP - Open Hardware edition
This project is a re-implementation of the popular HASwitchPlate sketch created by aderusha.
The [original HASwitchPlate][1] project uses a Wemos D1 mini and requires a Nextion/TJC HMI display.
This rewrite removes the Nextion/TJC requirement by using the [Littlev Graphics Library][2] on the MCU to drive a cheap commodity display.

This version also adds ESP32 support to take advantage of the additional hardware capabilities.


## Demo Screens

![Screenshot](https://raw.githubusercontent.com/fvanroie/hasp-lvgl/0.0.11/docs/img/sliders.png)
![Screenshot](https://raw.githubusercontent.com/fvanroie/hasp-lvgl/0.0.11/docs/img/buttons.png)
![Screenshot](https://raw.githubusercontent.com/fvanroie/hasp-lvgl/0.0.11/docs/img/mediaplayer.png)

## Features

| Feature                 | ESP8266 | ESP32
|-------------------------|---------|---------
| SPI display             | <ul><li>- [x] yes</li> | <ul><li>- [x] yes</li>
| Parallel display        | <ul><li>- [ ] no</li> | *tbd*
| PWM Screen dimming      | <ul><li>- [x] yes</li> | <ul><li>- [x] yes</li>
| Maximum Page Count      | 4       | 12
| Object Types / Widgets  | 14  | 15
| Dynamic Objects         | <ul><li>- [x] yes</li> | <ul><li>- [x] yes</li>
| [Lvgl Theme Support][3] | basic only | all themes
| [Custom .zi V5 font][4] | <ul><li>- [x] yes (latin1)</li> | <ul><li>- [x] yes (latin1)</li>
| [FontAwesome Icons][5]  | <ul><li>- [x] 1300+</li> | <ul><li>- [x] 1300+</li>
| PNG images              | <ul><li>- [ ] no</li> | *tbd*

## Cloning

Make sure to add the `--recursive` parameter when cloning the project. Otherwise git will not download the required submodules in the `/lib` subdirectory.

```bash
git clone --recursive https://github.com/fvanroie/hasp-lvgl
```

If you already cloned hasp-lvgl without the submodules, you can fetch the submodules seperately using:

```bash
git submodule update --init --recursive
```

## Getting Started

Check out the [wiki](https://github.com/fvanroie/hasp-lvgl/wiki) for how-to's, information and frequently asked questions.

[1]: https://github.com/aderusha/HASwitchPlate
[2]: https://github.com/littlevgl/lvgl
[3]: https://littlevgl.com/themes
[4]: https://github.com/fvanroie/HMI-Font-Pack/releases
[5]: https://fontawesome.com/cheatsheet/

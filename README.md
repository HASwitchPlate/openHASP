# HASP - Open Hardware edition
This project is a re-implementation of the popular HASwitchPlate sketch created by aderusha.
The [original HASwitchPlate][1] project uses a Wemos D1 mini and requires a Nextion/TJC HMI display.
This rewrite removes the Nextion/TJC requirement by using the [Littlev Graphics Library][2] on the MCU to drive a cheap commodity display.

This version also adds ESP32 and STM32 support to take advantage of the additional hardware capabilities.


## Demo Screens

![Screenshot](https://raw.githubusercontent.com/fvanroie/hasp-lvgl/0.0.11/docs/img/sliders.png) &nbsp; 
![Screenshot](https://raw.githubusercontent.com/fvanroie/hasp-lvgl/0.0.11/docs/img/buttons.png) &nbsp; 
![Screenshot](https://raw.githubusercontent.com/fvanroie/hasp-lvgl/0.0.11/docs/img/mediaplayer.png)

## Features

| Feature                 | ESP8266 | ESP32   | STM32F4
|-------------------------|---------|---------|----------
| SPI display             | :white_check_mark: yes | :white_check_mark: yes | :white_check_mark: yes
| Parallel display        | :x: no | :white_check_mark: yes | :white_check_mark: yes
| PWM Screen dimming      | :white_check_mark: yes | :white_check_mark: yes | :white_check_mark: yes
| Maximum Page Count      | 4       | 12 |
| Object Types / Widgets  | 14      | 15 | 15
| Dynamic Objects         | :white_check_mark: yes | :white_check_mark: yes | :white_check_mark: yes
| [Lvgl Theme Support][3] | basic only | all themes | tbd
| [Custom .zi V5 font][4] | :white_check_mark: yes (latin1) | :white_check_mark: yes (latin1) | no
| [FontAwesome Icons][5]  | :white_check_mark: 1300+ | :white_check_mark: 1300+ | no
| PNG images              | :x: no | :grey_question: tbd | :grey_question: tbd 
| Network                 | :white_check_mark: Wifi | :white_check_mark: Wifi | :grey_question: tbd

## Cloning

Make sure to add the `--recursive` parameter when cloning the project. Otherwise git will not download the required submodules in the `/lib` subdirectory.

```bash
git clone --recursive https://github.com/fvanroie/hasp-lvgl
```

If you already cloned hasp-lvgl without the submodules, you can fetch the submodules seperately using:

```bash
git submodule update --init --recursive
```

To build a defierent branch use:

```bash
git clone --recursive https://github.com/fvanroie/hasp-lvgl
cd hasp-lvgl
git checkout 0.1.0-dev
git submodule update --init --recursive
```

## Getting Started

Check out the [wiki](https://github.com/fvanroie/hasp-lvgl/wiki) for how-to's, information and frequently asked questions.

[1]: https://github.com/aderusha/HASwitchPlate
[2]: https://github.com/littlevgl/lvgl
[3]: https://littlevgl.com/themes
[4]: https://github.com/fvanroie/HMI-Font-Pack/releases
[5]: https://fontawesome.com/cheatsheet/

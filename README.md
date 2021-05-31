# openHASP firmware

[![GitHub Workflow Status](https://img.shields.io/github/workflow/status/HASwitchPlate/openHASP/PlatformIO%20CI?label=build%20status&logo=github&logoColor=%23dddddd)](https://github.com/HASwitchPlate/openHASP/actions)
[![GitHub release](https://img.shields.io/github/v/release/HASwitchPlate/openHASP?include_prereleases)](https://github.com/HASwitchPlate/openHASP/releases)
[![GitHub issues](https://img.shields.io/github/issues/HASwitchPlate/openHASP.svg)](http://github.com/HASwitchPlate/openHASP/issues)
[![Discord](https://img.shields.io/discord/538814618106331137?color=%237289DA&label=support&logo=discord&logoColor=white)][6]
[![contributions welcome](https://img.shields.io/badge/contributions-welcome-brightgreen.svg?style=flat)](#Contributing)
[![GitHub](https://img.shields.io/github/license/mashape/apistatus.svg)](https://github.com/HASwitchPlate/openHASP/blob/master/LICENSE)

Control your home-automation devices from a customizable touchscreen UI connected via MQTT.

This project is a re-implementation of the popular HASwitchPlate sketch created by aderusha.
The [HASPone][1] project uses a Wemos D1 mini and requires a Nextion/TJC HMI display.
This rewrite removes the Nextion/TJC hardware requirement by using the [Light and Versatile Graphics Library][2] on the MCU to drive a commodity display.

openHASP also adds ESP32 and STM32F4 support to take advantage of the additional hardware capabilities.


## Demo Screens

![Screenshot](https://haswitchplate.github.io/openHASP-docs/assets/images/screenshots/demo_switches_covers.png) &nbsp; 
![Screenshot](https://haswitchplate.github.io/openHASP-docs/assets/images/screenshots/demo_jaffa1.png) &nbsp; 
![Screenshot](https://haswitchplate.github.io/openHASP-docs/assets/images/screenshots/demo_mediaplayer.png)

## Features

| Feature                 | ESP8266 | ESP32   | STM32F4
|-------------------------|---------|---------|----------
| SPI display             | :white_check_mark: yes | :white_check_mark: yes | :white_check_mark: yes
| Parallel display        | :x: no | :white_check_mark: yes | :white_check_mark: yes
| PWM Screen dimming      | :white_check_mark: yes | :white_check_mark: yes | :white_check_mark: yes
| Maximum Page Count      | 4       | 12 | 12
| Object Types / Widgets  | 14      | 15 | 15
| Dynamic Objects         | :white_check_mark: yes | :white_check_mark: yes | :white_check_mark: yes
| [Lvgl Theme Support][3] | basic only | all themes | tbd
| [Custom .zi V5 font][4] | :white_check_mark: yes (latin1) | :white_check_mark: yes (latin1) | no
| [FontAwesome Icons][5]  | :white_check_mark: 1300+ | :white_check_mark: 1300+ | no
| PNG images              | :x: no | :grey_question: tbd | :grey_question: tbd 
| Network                 | :white_check_mark: Wifi | :white_check_mark: Wifi | :white_check_mark: Ethernet

## Cloning

Make sure to add the `--recursive` parameter when cloning the project. Otherwise git will not download the required submodules in the `/lib` subdirectory.

```bash
git clone --recursive https://github.com/HASwitchPlate/openHASP
```

If you already cloned openHASP without the submodules, you can fetch the submodules seperately using:

```bash
git submodule update --init --recursive
```

To build a defierent branch use:

```bash
git clone --recursive https://github.com/HASwitchPlate/openHASP
cd openHASP
git checkout 0.4.0
git submodule update --init --recursive
```

## Getting Started

Check out the [documentation](https://haswitchplate.github.io/openHASP-docs/) for how-to's, information and frequently asked questions.

Support
---------------------------
For support using openHASP, please join the [#openHASP channel][6] on Discord.

<script type="text/javascript" src="https://cdnjs.buymeacoffee.com/1.0.0/button.prod.min.js" data-name="bmc-button" data-slug="aktdCofU" data-color="#FFDD00" data-emoji=""  data-font="Cookie" data-text="Buy me a coffee" data-outline-color="#000000" data-font-color="#000000" data-coffee-color="#ffffff" ></script>


[1]: https://github.com/HASwitchPlate/HASPone
[2]: https://github.com/lvgl/lvgl
[3]: https://littlevgl.com/themes
[4]: https://github.com/fvanroie/HMI-Font-Pack/releases
[5]: https://fontawesome.com/cheatsheet/
[6]: https://discord.gg/VCWyuhF

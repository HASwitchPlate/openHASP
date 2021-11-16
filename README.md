# openHASP firmware

[![GitHub Workflow Status](https://img.shields.io/github/workflow/status/HASwitchPlate/openHASP/PlatformIO%20CI?label=build%20status&logo=github&logoColor=%23dddddd)](https://github.com/HASwitchPlate/openHASP/actions)
[![GitHub release](https://img.shields.io/github/v/release/HASwitchPlate/openHASP?include_prereleases)](https://github.com/HASwitchPlate/openHASP/releases)
[![GitHub issues](https://img.shields.io/github/issues/HASwitchPlate/openHASP.svg)](http://github.com/HASwitchPlate/openHASP/issues)
[![Discord](https://img.shields.io/discord/538814618106331137?color=%237289DA&label=discord&logo=discord&logoColor=white)][6]
[![contributions welcome](https://img.shields.io/badge/contributions-welcome-brightgreen.svg?style=flat)](#Contributing)
[![GitHub](https://img.shields.io/github/license/mashape/apistatus.svg)](https://github.com/HASwitchPlate/openHASP/blob/master/LICENSE)
[!["Buy Me A Coffee"](https://img.shields.io/badge/buy%20me%20a%20coffee-donate-yellow.svg)](https://www.buymeacoffee.com/aktdCofU)

Control your home-automation devices from a customizable touchscreen UI connected via MQTT.

This project is a re-implementation of the popular HASwitchPlate sketch created by aderusha.
The [HASPone][1] project uses a Wemos D1 mini and requires a Nextion/TJC HMI display.
This rewrite removes the Nextion/TJC hardware requirement by using the [Light and Versatile Graphics Library][2] on the MCU to drive a commodity display.

openHASP uses the ESP32 and STM32F4 to take advantage of the hardware capabilities.


## Demo Screens

![Screenshot](https://haswitchplate.github.io/openHASP-docs/0.6/assets/images/screenshots/demo_switches_covers.png) &nbsp; 
![Screenshot](https://haswitchplate.github.io/openHASP-docs/0.6/assets/images/screenshots/demo_jaffa1.png) &nbsp; 
![Screenshot](https://haswitchplate.github.io/openHASP-docs/0.6/assets/images/screenshots/demo_mediaplayer.png)

## Features

| Feature (v0.6.x)        | ESP32   | STM32F4
|-------------------------|---------|---------
| SPI display             | :white_check_mark: yes | :white_check_mark: yes
| Parallel display        | :white_check_mark: yes | :white_check_mark: yes
| PWM Screen dimming      | :white_check_mark: yes | :white_check_mark: yes
| Maximum Page Count      | 12      | 12
| [Object Types / Widgets][7]| 20      | 20
| Dynamic Objects         | :white_check_mark: yes | :white_check_mark: yes
| Theme Support           | yes     | yes
| MDI Icons               | :white_check_mark: yes | :white_check_mark: yes
| [PNG images][8]         | :white_check_mark: yes | :white_check_mark: yes
| Network                 | :white_check_mark: Wi-Fi | :white_check_mark: Ethernet


## Getting Started

Check out the [documentation](https://haswitchplate.github.io/openHASP-docs/) for how-to's, information and frequently asked questions.

Support
---------------------------
For support using openHASP, please join the [#openHASP channel][6] on Discord.

## [Buy me a coffee](https://www.buymeacoffee.com/aktdCofU)

[![Buy me a coffee](https://www.buymeacoffee.com/assets/img/custom_images/black_img.png)](https://www.buymeacoffee.com/aktdCofU)

[1]: https://github.com/HASwitchPlate/HASPone
[2]: https://github.com/lvgl/lvgl
[3]: https://littlevgl.com/themes
[4]: https://github.com/fvanroie/HMI-Font-Pack/releases
[5]: https://fontawesome.com/cheatsheet/
[6]: https://discord.gg/VCWyuhF
[7]: https://haswitchplate.github.io/openHASP-docs/0.6/design/objects#cheatsheet
[8]: https://haswitchplate.github.io/openHASP-docs/0.6/design/objects#image

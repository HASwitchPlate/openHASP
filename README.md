# openHASP firmware

[![GitHub Workflow Status](https://img.shields.io/github/workflow/status/HASwitchPlate/openHASP/Build%20branch?label=build%20status&logo=github&logoColor=%23dddddd)](https://github.com/HASwitchPlate/openHASP/actions)
[![GitHub release](https://img.shields.io/github/v/release/HASwitchPlate/openHASP?include_prereleases)](https://github.com/HASwitchPlate/openHASP/releases)
[![GitHub issues](https://img.shields.io/github/issues/HASwitchPlate/openHASP.svg)](http://github.com/HASwitchPlate/openHASP/issues)
[![Discord](https://img.shields.io/discord/538814618106331137?color=%237289DA&label=discord&logo=discord&logoColor=white)][3]
[![contributions welcome](https://img.shields.io/badge/contributions-welcome-brightgreen.svg?style=flat)](#Contributing)
[![GitHub](https://img.shields.io/github/license/mashape/apistatus.svg)](https://github.com/HASwitchPlate/openHASP/blob/master/LICENSE)
[!["PayPal"](https://img.shields.io/badge/Paypal-donate-00457C.svg?logo=paypal)](https://github.com/HASwitchPlate/openHASP#donate)

Control your home-automation devices from a customizable touchscreen UI connected via MQTT.

This project is a re-implementation of the popular HASwitchPlate sketch created by aderusha.
The [HASPone][1] project uses a Wemos D1 mini and requires a Nextion/TJC HMI display.
This rewrite removes the Nextion/TJC hardware requirement by using the [Light and Versatile Graphics Library][2] on the MCU to drive a commodity display.

openHASP uses the ESP32 and STM32F4 to take advantage of the hardware capabilities.


## Demo Screens

![Screenshot](https://www.openhasp.com/0.6/assets/images/screenshots/demo_switches_covers.png) &nbsp; 
![Screenshot](https://www.openhasp.com/0.6/assets/images/screenshots/demo_jaffa1.png) &nbsp; 
![Screenshot](https://www.openhasp.com/0.6/assets/images/screenshots/demo_mediaplayer.png)


## Getting Started

Check out the [documentation](https://www.openhasp.com/) for how-to's, information and frequently asked questions.</br>
For support using openHASP, please join the [#openHASP channel][3] on Discord.

## Donate

[![Paypal donation](https://img.shields.io/badge/Paypal-donate-00457C?style=for-the-badge&logo=paypal)][4]
[![Buy a coffee](https://img.shields.io/badge/Kofi-donate-FF5E5B?style=for-the-badge&logo=kofi)](https://ko-fi.com/openhasp)
[![Buy me a coffee](https://img.shields.io/badge/Buy_Me_a_Coffee-donate-FFDD00?style=for-the-badge&logo=buymeacoffee)](https://www.buymeacoffee.com/aktdCofU)

[1]: https://github.com/HASwitchPlate/HASPone
[2]: https://github.com/lvgl/lvgl
[3]: https://www.openhasp.com/discord
[4]: https://www.paypal.com/donate/?business=E76SN28JLZCXU&currency_code=EUR

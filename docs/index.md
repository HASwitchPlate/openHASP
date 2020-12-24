# Hasp-lvgl

## Purpose

Hasp-lvgl is a microcontroller firmware that runs on ESP and STM32F4 boards using an off-the-shelve touch display.
You can use the hasp-lvgl firmware to create a custom touchscreen user interface.
It can display information received over mqtt and you can create on-screen objects to interact with your home automation system,
like touch buttons, switches, LEDs and more...

This project is a re-implementation of the popular HASwitchPlate sketch created by aderusha. The original HASwitchPlate project uses a Wemos D1 mini and requires a Nextion/TJC HMI display. This rewrite removes the Nextion/TJC requirement by using the Littlev Graphics Library on the MCU to drive a cheap commodity display.

## Requirements

To run the firmware, you only need a compatible [microcontroller](01-hardware?id=recommended-boards) and [touch display](01-hardware?id=recommended-display).

## Support

For support using hasp-lvgl, please join the Discord channel
![Discord](https://img.shields.io/discord/538814618106331137?color=%237289DA&label=%23hasp-lvgl&logo=discord&logoColor=white)

<!--
**If you enjoy this software, please consider [supporting me](https://www.paypal.me/netwizeBE) for developing and maintaining it.**

[![Support via PayPal](https://cdn.jsdelivr.net/gh/twolfson/paypal-github-button@1.0.0/dist/button.svg)](https://www.paypal.me/netwizeBE)
-->
# Hasp-lvgl Documentation

## Purpose

Hasp-lvgl is a microcontroller firmware that can run on ESP and STM32F4 with an off-the-shelve touch display.
You can use the hasp-lvgl firmware to create a custom touchscreen user interface.
It can display information and you can create objects like touch buttons, switches and LEDs on the touchscreen to interact with your home automation system.

The communication is done over the network via MQTT.

This project is a re-implementation of the popular HASwitchPlate sketch created by aderusha. The original HASwitchPlate project uses a Wemos D1 mini and requires a Nextion/TJC HMI display. This rewrite removes the Nextion/TJC requirement by using the Littlev Graphics Library on the MCU to drive a cheap commodity display.

This version also adds ESP32 and STM32F4 support to take advantage of the additional hardware capabilities.

## Requirements

To run the firmware, you only need a compatible microcontroller and touch display.

**If you enjoy this software, please consider [supporting me](https://www.paypal.me/netwizeBE) for developing and maintaining it.**

[![Support via PayPal](https://cdn.jsdelivr.net/gh/twolfson/paypal-github-button@1.0.0/dist/button.svg)](https://www.paypal.me/netwizeBE)

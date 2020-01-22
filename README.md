# HASP - Open Hardware edition
This is a re-implementation of the popular HASwitchPlate sketch created by aderusha.
The [original HASwitchPlate][1] project uses a Wemos D1 mini and requires a Nextion/TJC HMI display.
This fork removes the Nextion/TJC requirement by using the Littlev Graphics Library on the MCU to drive a commodity display.

This version also adds ESP32 support to take advantage of the additional hardware capabilities.

[1]: https://github.com/aderusha/HASwitchPlate

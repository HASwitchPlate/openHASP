# openHASP Changelog

## v0.7.0

!!! THE PARTITION SCHEME OF THE INTERNAL FLASH HAS CHANGED !!!

### Objects
<!-- ? Support for State and Part properties -->
- `action` and `swipe` can now be set to any command
- Set default line_width of new `line` objects to 1
- Allow line and block comments in pages.jsonl
- HASP theme: Toggle objects now use the secondary color when they are in the toggled state.

### Fonts
- Firmware files include the bitmapped font sizes 12, 16, 24 and 32pt
- Use embedded TrueType font for other font sizes (PSram highly recommended)
- Add glyphs from character sets Cyrillic, Latin-2, Greek and Viernamese to default fonts

### Web UI
- Update Web UI to petite-vue app
- Redesigned the File Editor
<!-- - _Selectable dark/light theme?_ -->

### Services
- Change MQTT client from _PubSubClient_ to asynchronic Espressif _esp_mqtt_ client
- Make the MQTT topics configurable
- MQTT discovery now uses a subtopic of `hasp/discovery`. Discovery requires version 0.7.x of the Custom Component.
- Add service start/stop mqtt
- Add SimpleFTPServer to easily upload and download files to the plate *(one simultanious connection only)*
- Add service start/stop ftp
- Add configuration for NTP servers and timezone

### Devices
- Add GS-T3E Smart Panel
- Add Lilygo Ttgo Lily Pi ESP32
- Add Makerfabs ESP32-S3 SPI
- Add Sunton ESP32-S3 TFT 4.3", 5.0" and 7.0"
- Add Sunton ESP32-2432S028R ESP32-3248S035C ESP32-3248S035R
- Add support for Wireless-Tag WT32-SC01 Plus and WT-86-32-3ZW1

## Bug fixes
- Fix for first touch not working properly
- Add button GPIOs to input discovery message

### Architecture
- Moved to Tasmota Arduino 2.0.6 and ESP-IDF 4.4.3 (thanks @Jason2866)
- Add Arduino-GFX display driver
- Add support for ESP32-S3 devices
- Deprication of support for ESP32-S2 devices due to lack of sRAM

Updated libraries to ArduinoJson 6.20.1, ArduinoStreamUtils 1.7.0, TFT_eSPI 2.5.0, LovyanGFX 1.1.2 and SimpleFTPServer 2.1.5


## v0.6.3

### Commands
- Additional `idle` parameters now accept `off`, `short` and `long`
- Add `sensors` command to trigger the sensors state message
- Run `L:/boot.cmd` when the plate is (re)booted if the script exists
- You can now use the `pXbY.jsonl` command to update multiple object properties at once (thanks @nagyrobi)

### Objects
- All objects have a custom `tag` property which can contain arbitrary JSON data *(or numbers or text)* (thanks @nagyrobi)
- `img.src` now accepts both `http` and `https` urls (thanks @htvekov)
- `img.src` now accepts `png` and `binary` image urls, PSram is *highly* recommended
- `img.src` now accepts 16-bit BMP files stored in flash

### Web UI
- Updated to modern responsive design *(requires JavaScript)*
- Add `/api/info/` and `/api/config/*/` endpoints
- Allow for a customizible `vars.css`, `style.css`, `script.js` and `edit.htm`
- Display a message when the configuration is changed and a reboot is needed
- Add checkbox for backlight inversion to Display settings (thanks @wolffman122)
- Add checkbox to toggle ANSI codes #261 (thanks @geiseri)
- Allow firmware upgrade/downgrade in AP mode

### GUI
- `antiburn` displays random pixels *aka.* white noise
- Hide cursor during `antiburn` and `idle` if the pointer is enabled

### MQTT
-  Remember last `page` id of `jsonl` messages. Sending multiple messages now behaves like `jsonl` files (thanks @arovak)

### Fonts
- Use TrueType fonts from flash, PSram is *highly* recommended (thanks to @s-hadinger)
- Custom LVGL binary fonts can be read from flash and loaded into memory, PSram is *highly* recommended
- Font ID is replaced by `namexx` *(where `xx` is the font size)* but the previous Font IDs still work for backwards compatibility
- Added build option for Vietnamese character set (thanks @kydang789)
- ⚠️ **Breaking:** Removed defunct .zi font support!

### Bug fixes
- Fixed Lanbon L8 v1.17 PCB screen corruption #316 (thanks @DJBenson)- Fix for screenshots not showing properly in Safari on macOS/iOS (thanks @masto)
- Fix bug that would not accept `on` state for setting output GPIOs #275 (thanks @freshnas and @cerietke)
- Fix a bug in `dropdownlist` were `close` method performed `open` instead #299 (thanks @htvekov)
- Fix `src` bug in `img` objects that could corrupt images sent over http (thanks @htvekov)
- Fix screen dimensions in `statusupdate` message, taking into account current orientation #278 (thanks @kquinsland)
- Fix for HTTP password that could be overwritten by 8 asterisks when it was not changed in the web UI
- Fixed MQTT hostname limit *again*... #304 (thanks @fake-name)
- Fixed firmware upgrade from URL #300 (thank @nagyrobi)
- Fixed a bug that prevented Wifi from connecting to different BSSID #330 (thanks @Braehead)
- Syslog message format fixes and improvements #285 (thanks @geiseri)

### Custom component
- Expose `antiburn` and `page` in the CC (thanks @dgomes)
- Expose the device URL in discovery message and CC

### Architecture
- Moved to Tasmota Arduino 2.0.3 with native LittleFS library (thanks @Jason2866) and ESP-IDF 4.4.1 fixes for FragAttacks CVEs (thanks @nagyrobi)
- Add support for ESP32-S2 devices
- ⚠️ **Breaking:** Removed support for ESP8266!

Updated libraries to ArduinoJson 6.19.4, ArduinoStreamUtils 1.6.3, AceButton 1.9.2, TFT_eSPI 2.4.61, LovyanGFX 0.4.17 and Adafruit STMPE610 1.1.4



## v0.6.2

### Initial Setup
- Add Captive Portal to first time setup (thanks @AndreTeixeira1998)
- Create default `pages.jsonl`, `online.cmd` and `offline.cmd` files if they don't exist (thanks @nagyrobi)

### Objects
- Enable `click` by default on `image` object
- Add `type` to `spinner` object
- Add `zoom`, `angle`, `pivot_x`, `pivot_y` and `antialias` attributes to `image` object
- Allow url as `src` of `image` object for raw webimages and from push image service from the CC (thanks @dgomes and @nagyrobi)
- Use `L:/file.png` instead of `/littlefs/file.png` for image paths, `/littlefs/` still works for backwards compatibility

### Objects
- Add `antiburn` command to prevent static parts of the screen to create a *ghosting* effect in some LCDs or conditions

### Devices
- Add Analog touch driver for Unoshield displays (thanks @wesleygas)
- Add AZ-Touch MOD ESP32 with 2.4" or 2.8"
- Add Lilygo®Ttgo Pi ESP32 with TFT 3.5"
- Add Waveshare ESP32 One development board with Rpi MHS4001(B) or Waveshare RPi(C) LCD display
- Add D1-R32 ESP32 development board with with Waveshare ILI9486 Touch Shield

### Bug fixes
- Fix bug that caused a crash when both `btnmatrix` and `msgbox` where used (thanks @AndreTeixeira1998)
- Fix L8-HD dimmer brightness initialization after a reboot (thanks @Stupco)
- Keep last dimmer value when toggling dimmer state on/off 
- Fix configurable mqttPort (thanks @Qargh)
- Fix opaque background of `spinner` object in HASP theme (thanks @nagyrobi)

Updated libraries to AceButton 1.9.1 and ArduinoJson 6.18.5


## v0.6.1

### Commands
- Add `run` command to execute script files (`.cmd` or `.jsonl`)
- Add `unzip` command for __no-compression__ zip files
- Add `service` command to start/stop a service

### Wifi
- Don't reboot the plate anymore after prolonged wifi connection lost
- Run `/online.cmd` or `/offline.cmd` script when the wifi status changed

### Objects
- Add new *[line](https://openhasp.haswitchplate.com/0.6.1/design/objects/#line)* object
- Add `val` to *[btnmatrix](https://openhasp.haswitchplate.com/0.6.1/design/objects/#button-matrix)* when `one_select` is set
- Cache up to 20 *[images](https://openhasp.haswitchplate.com/0.6.1/design/objects/#image)* in PSram when available
- Improve precision on the *[linemeter](https://openhasp.haswitchplate.com/0.6.1/design/objects/#line-meter)* scales
- Fix *[dropdown](https://openhasp.haswitchplate.com/0.6.1/design/objects/#dropdown-list)* redraw bug

### Devices
- Fix [L8-HD dimmer](https://openhasp.haswitchplate.com/0.6.1/devices/lanbon-l8/) not responding correctly to mqtt after a reboot
- Add [M5Stack Core2](https://openhasp.haswitchplate.com/0.6.1/devices/m5stack-core2/) backlight dimming
- Add [Yeacreate Nscreen32](https://openhasp.haswitchplate.com/0.6.1/devices/yeacreate-nscreen32/)
- Add [Makerfabs ESP32 TFT Touch](https://openhasp.haswitchplate.com/0.6.1/devices/makerfabs-tft-touch/) Capacitive

### Fonts
- [Additional characters](https://openhasp.haswitchplate.com/0.6.1/design/fonts/#ascii): `²` (squared) and `³` (cubed)
- [Additional icons](https://openhasp.haswitchplate.com/0.6.1/design/fonts/#built-in-icons): recycle-variant and additional weather icons
- Use latin1 as default charset on [WT32-SC01](https://openhasp.haswitchplate.com/0.6.1/devices/wt32-sc01/)
- Add [Greek font](https://openhasp.haswitchplate.com/0.6.1/design/fonts/#greek)

### Compiling
- Allow custom bootlogo
- Selectively start http, telnet or console at boot
- Updated AceButton to 1.9.0 and ArduinoJson to 6.18.3


## v0.6.0

### Commands:
- Obsolete `dim` and `light` commands, use `backlight` command instead
- Add `discovery` command to facilitate HA CC discovery
- Add `idle` command to retreive idle state, replaces `wakeup` command
- Updated `moodlight` command with brightness support
- Rewrite `outputX` and add `inputX` command

### Objects:
- `lmeter` object renamed to `linemeter`
- `align` values are now `left`, `right`, `center` instead of numbers *(numbers can still be used)*
- Added `ext_click_h` and `ext_click_v` attributes to extend the clickable area
- Added `clear` method to remove only the child objects from an object
- New `tabview`, `tab`, `calendar` and `msgbox` objects
- Add `img` object with png support *(needs PSram for any sizable images)*
- Added missing properties of the `spinner` object

### Web UI:
- Updated GPIO configuration pages
- Include a File Editor on ESP32: Create, Edit, Upload, Delete and Apply pages.jsonl (Thanks Cossie)
- Cache css, favicon and edit.htm.gz in the browser
- New Information page layout, including MQTT message counters for sent, received and failed

### MCU:
- Allow longer wifi ssid and password
- Update partition boundaries to use *all* remaining space for lfs filesystem
- Speed improvement: keep LVGL functions in fast memory & reduce overhead in main loop
- Use ascii characterset for WT32-SC01 due to limited flash size

### GPIO rewrite:
- Better handling of gpios
- Support for Lanbon L8-HD dimmer (EU and AU version, thanks to @Stupco)
- Add moodlight brightness support

### Broadcast and Discovery topic (HASP_USE_BROADCAST and HASP_USE_DISCOVERY)
- Allow dynamic configuration of HA entities using the [openHASP Custom Component](https://github.com/HASwitchPlate/openHASP-custom-component/releases/) (thanks @dgomes)
- Add Manufacturer and Model to statusupdate

Updated libraries to lvgl 7.11.0, ArduinoJson 6.18.0 and TFT_eSPI 2.3.70

### Internationalization:
- Added French language
- Added Portuguese language (thanks @AndreTeixeira1998 and @dgomes)
- Added Spanish language (thanks @altersis)

## v0.5.1

- Restore broken `config/submodule` topics
- Don't show warning on `comment` attribute
- Patched a potential memory leak
- Added an automatic lv_mem_defrag to free up lvgl memory
- Allow selection of fonts via user_config_override
- Allow for longer mqtt configuration strings in user_config_override
- Allow default HASP_GPIO_TEMPLATE configuration in user_config_override
- Add `manufacturer` and `model` to statusupdate
- Fixed `enabled` attribute and added `click` attribute

## v0.5.0

Name changed to openHASP - https://openhasp.haswitchplate.com/
> When using HomeAssistant also update the [openHASP Custom Component](https://github.com/HASwitchPlate/openHASP-custom-component/releases/tag/0.5.0)

- Switch built-in icons from FontAwesome to MaterialDesign icons #139
- Add built option for other character sets then latin1
- Built-in Font sizes dependent on screen size
- Add `swipe` property to switch pages
- Add `action` property for local page navigation
- Add `back`, `prev`, `next` attributes to pages #114
- JSON Serialize text in payloads containing text attributes #140
- Add az-touch-mod-esp32_ili9341 config and allow for TFT_BACKLIGHT_ON set to LOW #131
- Add [FreeTouchDeck](https://openhasp.haswitchplate.com/0.5/#devices/freetouchdeck/) and [ESP32-Touchdown](https://openhasp.haswitchplate.com/0.5/#devices/esp32-touchdown/) configs
- Add roller `mode` `infinite` attribute
- Add btnmatrix `toggle` and `one_check` attributes
- Rework all event handlers to reduce update events and prevent race condition #119 *(events have changed!)*
- Add ability to style the selected part of roller object
- Add `scan_method = WIFI_ALL_CHANNEL_SCAN` for ESP32, improving multi-AP connection
- Add warning `objid` property is obsolete, use `obj` instead
- Add warning `txt` property is obsolete, use `text` instead
- Add dark theme build option for web UI
- Update lvgl and tft_espi library version
- Update maximum backlight_level from 100 to 255
- Set default `HASP_LOG_LEVEL=LOG_LEVEL_TRACE`

## v0.4.0

**Note:** The partition scheme has changed and you will need to reflash the device over serial first.

*All data will be erased, so make sure to backup your pages.jsonl, config.json and fonts.*

Changes:
- Provide all-in-one binary files for ESP32 to flash a device over serial using a single .bin file
- Allow long wifi passwords (#71 thanks @nagyrobi)
- Wakeup screen on first touch (#80)
- Reduce slider events (#88)
- Update events to accommodate the [HA Custom Component](https://github.com/HASwitchPlate/openHASP-custom-component) (by @dgomes)
- Remove HA auto-discovery in favor of the HA Custom Component
- Add `clearpage all` command option
- Add local page navigation and transitions
- Add [scale properties](https://openhasp.haswitchplate.com/0.5/#styling/#scale)
- Add `config/gpio` command
- Allow for timezone setting in user_config_override.h (thanks @arovak)
- Start localizations for NL, HU and RO (thanks @nagyrobi)
- New prebuild devices:
  - esp32-touchdown (thanks @joelhaasnoot and @dustinwatts)
  - huzzah featherwing 2.4" and 3.5" (thanks @arovak)
  - m5stack-core2
- Fix zi font crash bug
- Update setting min/max attributes (#103 thanks @arovak)
- Native builds for Windows, MacOS and Linux (thanks to @thouters and @dgomes)

## v0.3.4

The webserver was not properly started at initial setup, when the device was in AP mode.

## v0.3.3

- Change `txt` to `text`. `txt` is now obsoleted.
- Initial support for moodlight e.g. Lanbon L8

## v0.3.2

- Add Lanbon L8
- Add WT32-SC01

## v0.3.1

- Fixes wifi connection issue in OOBE
- Fixes memory leak in value_str

## v0.3.0

First release of compiled .bin files

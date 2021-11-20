# openHASP Changelog

## v0.7.0

### Fonts
- Use FreeType fonts from flash
- Use LVGL binary fonts from flash, loaded into PSram
- Removed defunct .zi font support
- Breaking: The UTF codes for the built-in icons have changed from the previous list!
- _Breaking: Font ID is replaced by xxxx?_

### Web UI
- Updated to modern responsive design
- Allow for a customizible `vars.css`, `style.css`, `script.js` and `edit.htm`
- _Selectable dark/light theme?_

### Commands
- Hide cursor during `antiburn` and `idle` if the pointer is enabled

### Custom component
- Expose `antiburn` for the CC
- Expose the device URL in discovery messages

### Architecture
- Moved to Arduino 2.0 with native LittleFS library
- Moved to ESP-IDF 4.4 with fix for FragAttacks CVEs
- Prepare support for ESP32S2
- Breaking: Removed support for ESP8266


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
- Add Arduitouch MOD ESP32 with 2.4" or 2.8"
- Add Lilygo®Ttgo Pi ESP32 with TFT 3.5"
- Add Waveshare ESP32 One development board with Rpi MHS4001(B) or Waveshare RPi(C) LCD display
- Add D1-R32 ESP32 development board with with Waveshare ILI9486 Touch Shield

### Bug fixes
- Fix bug that caused a crash when both `btnmatrix` and `msgbox` where used (thanks @AndreTeixeira1998)
- Fix L8-HD dimmer brightness initialization after a reboot (thanks @Stupco)
- Keep last dimmer value when toggling dimmer state on/off 
- Fix configurable mqttPort (thanks @Qargh)
- Fix opaque background of `spinner` object in HASP theme (thanks @nagyrobi)

Updated AceButton to 1.9.1 and ArduinoJson to 6.18.5


## v0.6.1

### Commands
- Add `run` command to execute script files (`.cmd` or `.jsonl`)
- Add `unzip` command for __no-compression__ zip files
- Add `service` command to start/stop a service

### Wifi
- Don't reboot the plate anymore after prolonged wifi connection lost
- Run `/online.cmd` or `/offline.cmd` script when the wifi status changed

### Objects
- Add new *[line](https://haswitchplate.github.io/openHASP-docs/0.6.1/design/objects/#line)* object
- Add `val` to *[btnmatrix](https://haswitchplate.github.io/openHASP-docs/0.6.1/design/objects/#button-matrix)* when `one_select` is set
- Cache up to 20 *[images](https://haswitchplate.github.io/openHASP-docs/0.6.1/design/objects/#image)* in PSram when available
- Improve precision on the *[linemeter](https://haswitchplate.github.io/openHASP-docs/0.6.1/design/objects/#line-meter)* scales
- Fix *[dropdown](https://haswitchplate.github.io/openHASP-docs/0.6.1/design/objects/#dropdown-list)* redraw bug

### Devices
- Fix [L8-HD dimmer](https://haswitchplate.github.io/openHASP-docs/0.6.1/devices/lanbon-l8/) not responding correctly to mqtt after a reboot
- Add [M5Stack Core2](https://haswitchplate.github.io/openHASP-docs/0.6.1/devices/m5stack-core2/) backlight dimming
- Add [Yeacreate Nscreen32](https://haswitchplate.github.io/openHASP-docs/0.6.1/devices/yeacreate-nscreen32/)
- Add [Makerfabs ESP32 TFT Touch](https://haswitchplate.github.io/openHASP-docs/0.6.1/devices/makerfabs-tft-touch/) Capacitive

### Fonts
- [Additional characters](https://haswitchplate.github.io/openHASP-docs/0.6.1/design/fonts/#ascii): `²` (squared) and `³` (cubed)
- [Additional icons](https://haswitchplate.github.io/openHASP-docs/0.6.1/design/fonts/#built-in-icons): recycle-variant and additional weather icons
- Use latin1 as default charset on [WT32-SC01](https://haswitchplate.github.io/openHASP-docs/0.6.1/devices/wt32-sc01/)
- Add [Greek font](https://haswitchplate.github.io/openHASP-docs/0.6.1/design/fonts/#greek)

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

### Updated lvgl to 7.11.0, ArduinoJson to 6.18.0 and TFT_eSPI to 2.3.70

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

Name changed to openHASP - https://haswitchplate.github.io/openHASP-docs/
> When using HomeAssistant also update the [openHASP Custom Component](https://github.com/HASwitchPlate/openHASP-custom-component/releases/tag/0.5.0)

- Switch built-in icons from FontAwesome to MaterialDesign icons #139
- Add built option for other character sets then latin1
- Built-in Font sizes dependent on screen size
- Add `swipe` property to switch pages
- Add `action` property for local page navigation
- Add `back`, `prev`, `next` attributes to pages #114
- JSON Serialize text in payloads containing text attributes #140
- Add arduitouch-esp32_ili9341 config and allow for TFT_BACKLIGHT_ON set to LOW #131
- Add [FreeTouchDeck](https://haswitchplate.github.io/openHASP-docs/#devices/freetouchdeck/) and [ESP32-Touchdown](https://haswitchplate.github.io/openHASP-docs/#devices/esp32-touchdown/) configs
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
- Update events to accommodate the [HA Custom Component](https://github.com/dgomes/hasp-lvgl-custom-component) (by @dgomes)
- Remove HA auto-discovery in favor of the HA Custom Component
- Add `clearpage all` command option
- Add local page navigation and transitions
- Add [scale properties](https://fvanroie.github.io/hasp-docs/#styling/#scale)
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
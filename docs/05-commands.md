Commands are not related to an object, but can get or set global properties or invoke system commands on the device.

Commands can be issued via the Serial Commandline, Telnet Commandline or MQTT.
For MQTT, use the `hasp/<platename>/command` topic with payload `<keyword> <parameter(s)>`

Here is a list of all the recaognized command keywords:

## Pages

`page` [0-11]

Switches the display to show the objects from a diferent page.

`clearpage` [0-11,254]

Deletes all objects on a given page. If no page number is specified, it clears the current page.

To delete individual objects, you can issue the `p[x].b[y].delete` command.

## Backlight

`dim` [0-100] (alias: `brightness`)

Sets the level of the backlight from 0 to 100%, where 0% is off and 100% is full brightness.

Example: `dim 50` sets the display to half the brightness.

Tip: this can be used in conjunction with the idle event e.g. to dim the backlight after a short period of inactivity.

`light`

Switches the backlight on or off, independent of the set dim level.
Turning the backlight on will restore the brightness to the previous dim level.

Example: `light on` acepted values: on/off, true/false, 0/1, yes/no

Tip: this can be used in conjunction with the idle event, e.g. to turn the backlight off after a long period of inactivity.

?> The `dim`and `light` command depends on a GPIO pin to be connected to control the the TFT_LED backlight via a transistor.

`wakeup`

Clears the idle state of the device and publishes an `state/idle = OFF` status message. It resets the idle counter as if a touch event occured on the devide. This is helpfull e.g. when you want to wake up the display when an external event has occured, like a PIR motion sensor.

## System commands

`calibrate`

Start on-screen touch calibration.

?> You need to issue a soft reboot command to save the new calibration settings. If you do a hard reset of the device, the calibration settings will be lost.

`screenshot`

Saves a picture of the current screen to the flash filesystem. You can retrieve it via http://&gt;ip-address&lt;/screenshot.bmp
This can be handy for bug reporting or documentation.

The previous screenshot is overwritten.

`statusupdate`

Reports the status of the MCU. The response will be posted to the state topic:
```json
    "statusupdate": {
        "status": "available",
        "espVersion": "0.0.6",
        "espUptime": 124,
        "signalStrength": -72,
        "haspIP": "10.1.0.148",
        "heapFree": 5912,
        "heapFragmentation": 7,
        "espCore": "2_6_3"
    }
```

`reboot` (alias: `restart`)

Saves any changes in the configuration file and reboots the device.

`factoryreset`

Clear the filesystem and eeprom and reboot the device in its initial state.

Warning: There is no confirmation prompt nor undo function!

## Configuration Settings

### Wifi

`ssid`

Sets network name of the access point to connect to.

`pass`

Sets the optional password for the access point to connect to.

### MQTT

`hostname`

Sets the hostname of the device and mqtt topic for the node to `hasp/<hostname>/`

`mqtthost`

Sets the hostname of the mqtt broker.

`mqttport`

Sets the port of the mqtt broker.

`mqttuser`

Sets the optional username for the mqtt broker.

`mqttpass`

Sets the optional password for the mqtt broker.

### Config/xxx

You can get or set the configuration of a hasp-lvgl submodule in json format.
To get the configuration, the command `config/&gt;submodule&lt;`. 
The result will be published to `hasp/plate35/state/config`. Passwords will be omited from the result.

```
config/wifi
config/mqtt
config/http
config/mdns
config/hasp
config/gui
config/debug
```

To update the configuration simple issue the same command `config/&gt;submodule&lt;` with updated json payload.

## Multiple Commands

`json`

When you want to execute multiple commands in one payload, you can use the json command to create an array of commands.

Each command is an element in this array of strings:

```json
["page 5","dim 50","light on","statusupdate"]
```

The commands are interpreted and processed sequentially.
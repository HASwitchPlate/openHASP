## Download the firmware

Go to the releases page on Github to download the latest hasp-lvgl binaries.

[<i class="fas fa-download"></i> Hasp-lvgl Releases](https://github.com/fvanroie/hasp-lvgl/releases){: .btn .btn--info}

There are currently 2 download options, pick the one appropriate for your hardware:
- hasp-lvgl-0.2.0-esp32_ili9341_spi.bin + boot files
- hasp-lvgl-0.2.0-esp8266_ili9341_spi.bin

?> If no precompiled firmware file is available for your board you can configure, compile and upload the firmware yourself using PlatformIO.


## Install the firmware

### Flash ESP32

When flashing the ESP32 for the first time, you need to install a bootloader, partitionscheme and application loader:
```shell
esptool.py --port "COM1" erase_flash
esptool.py --port "COM1" write_flash 0x1000 bootloader_dio_40m.bin --flash_mode dio --flash_freq 40m
esptool.py --port "COM1" write_flash 0x8000 partitions.bin
esptool.py --port "COM1" write_flash 0xe000 boot_app0.bin
```

Change `COM1` to the correct port on your computer.

then flash the actual firmware:

```shell
esptool.py -p "COM1" --baud 921600 write_flash 0x10000 d1-mini-esp32_ili9341_<version>.bin
```

or all previous steps in one long commandline:

```shell
esptool.py -p "COM1" --baud 921600 --before default_reset --after hard_reset write_flash --flash_mode dio --flash_freq 40m --flash_size detect 0x1000 bootloader_dio_40m.bin 0x8000 partitions.bin 0xe000 boot_app0.bin 0x10000 d1-mini-esp32_ili9341_<version>.bin
```

### Flash ESP8266

Unlike the ESP32, for ESP8266 you only need one single `.bin` file:

#### Using Tasmotizer (Windows)

#### Using esp-tool.py

```shell
esptool.py -p "COM1" write_flash --flash_mode qio --flash_size 4m 0x0 d1-mini-esp8266_ili9341_<version>.bin
```

Change `COM1` to the correct port on your computer and `4m` to the correct size of the internal flash chip.

----------------------------------------------------------------------------------

### STM32F407 devEbox

?> There is no precompiled firmware file available for STM32F4 boards. You will need to configure, compile and upload the firmware yourself using PlatformIO.

#### Using Serial

- Connect your serial TTL adapter RX and TX pins to PA9 and PA10 of the devEbox.
- Place the boot jumpers into programming mode
- Reset the board.
- Upload the firmware using:

#### Using DFU (USB)

- Connect your serial TTL adapter RX and TX pins to PA9 and PA10 of the devEbox.
- Place the boot jumpers into programming mode
- Reset the board.
- Upload the firmware using:

#### Using ST Link (USB)

- Install ST Link software
- Connect the devEbox using the USB port
- Launch ST Link
- Select the hasp-lvgl-0.2.0-stm32f407_devEbox_3.2_ili9341_fsmc.bin file
- Flash the firmware to the board
/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_TFT_DEFINES_H
#define HASP_TFT_DEFINES_H

#if defined(ARDUINO)

#if !defined(TOUCH_CS) && !defined(USER_SETUP_LOADED)
#define TOUCH_CS -1
#endif

#ifndef TOUCH_IRQ
#define TOUCH_IRQ -1
#endif

#ifndef TOUCH_RST
#define TOUCH_RST -1
#endif

#ifndef TFT_DMA_CHANNEL
#define TFT_DMA_CHANNEL 0
#endif

#ifndef TFT_SPI_MODE
#define TFT_SPI_MODE 0
#endif

#ifndef TFT_SPI_HOST
#define TFT_SPI_HOST 3
#endif

#ifndef TFT_MOSI
#define TFT_MOSI -1
#endif
#ifndef TFT_MISO
#define TFT_MISO -1
#endif
#ifndef TFT_SCLK
#define TFT_SCLK -1
#endif
#ifndef TFT_BUSY
#define TFT_BUSY -1
#endif
#ifndef TFT_CS
#define TFT_CS -1
#endif
#ifndef TFT_RST
#define TFT_RST -1
#endif

#ifndef SPI_FREQUENCY
#define SPI_FREQUENCY 40000000
#endif

#ifndef TFT_D0
#define TFT_D0 -1
#endif
#ifndef TFT_D1
#define TFT_D1 -1
#endif
#ifndef TFT_D2
#define TFT_D2 -1
#endif
#ifndef TFT_D3
#define TFT_D3 -1
#endif
#ifndef TFT_D4
#define TFT_D4 -1
#endif
#ifndef TFT_D5
#define TFT_D5 -1
#endif
#ifndef TFT_D6
#define TFT_D6 -1
#endif
#ifndef TFT_D7
#define TFT_D7 -1
#endif
#ifndef TFT_D8
#define TFT_D8 -1
#endif
#ifndef TFT_D9
#define TFT_D9 -1
#endif
#ifndef TFT_D10
#define TFT_D10 -1
#endif
#ifndef TFT_D11
#define TFT_D11 -1
#endif
#ifndef TFT_D12
#define TFT_D12 -1
#endif
#ifndef TFT_D13
#define TFT_D13 -1
#endif
#ifndef TFT_D14
#define TFT_D14 -1
#endif
#ifndef TFT_D15
#define TFT_D15 -1
#endif
#ifndef TFT_RD
#define TFT_RD -1
#endif
#ifndef TFT_WR
#define TFT_WR -1
#endif
#ifndef TFT_DC
#define TFT_DC -1
#endif
#ifndef SPI_READ_FREQUENCY
#define SPI_READ_FREQUENCY 0
#endif
#ifndef TFT_OFFSET_X
#define TFT_OFFSET_X 0
#endif
#ifndef TFT_OFFSET_Y
#define TFT_OFFSET_Y 0
#endif
#ifndef TFT_OFFSET_ROTATION
#define TFT_OFFSET_ROTATION 0
#endif
#ifndef TOUCH_OFFSET_ROTATION
#define TOUCH_OFFSET_ROTATION 0
#endif
#ifndef I2C_TOUCH_PORT
#define I2C_TOUCH_PORT 0
#endif

#endif // Arduino
#endif // HASP_TFT_DEFINES_H
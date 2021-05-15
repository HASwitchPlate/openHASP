/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#if !(defined(WINDOWS) || defined(POSIX))

#include "hasplib.h"
#include "hasp_oobe.h"
#include "sys/net/hasp_network.h"
#include "dev/device.h"
#include "drv/hasp_drv_touch.h"
#include "ArduinoLog.h"

#if HASP_USE_CONFIG > 0
#include "hasp_debug.h"
#endif

#if HASP_USE_CONFIG > 0
#include "hasp_config.h"
#include "hasp_gui.h"
#endif

bool isConnected;
uint8_t mainLoopCounter        = 0;
unsigned long mainLastLoopTime = 0;
uint8_t statLoopCounter        = 0;

void setup()
{
    //   hal_setup();

    haspDevice.init();

    /****************************
     * Storage initializations
     ***************************/
#if HASP_USE_EEPROM > 0
    eepromSetup(); // Don't start at boot, only at write
#endif

    // #if HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0
    //     filesystemSetup();  // FS mount is done in configSetup()
    // #endif

    // #if HASP_USE_SDCARD > 0
    //     sdcardSetup();
    // #endif

    /****************************
     * Read & Apply User Configuration
     ***************************/
#if HASP_USE_CONFIG > 0
    configSetup(); // also runs  debugSetup() and debugStart()
#endif

    dispatchSetup(); // before hasp and oobe, asap after logging starts
    guiSetup();

#if HASP_USE_CONFIG > 0
    if(!oobeSetup())
#endif
    {
        haspSetup();
    }

    /****************************
     * Apply User Configuration
     ***************************/

#if HASP_USE_MQTT > 0
    mqttSetup(); // Load Hostname before starting WiFi
#endif

#if HASP_USE_GPIO > 0
    gpioSetup();
#endif

#if HASP_USE_WIFI > 0 || HASP_USE_ETHERNET > 0
    networkSetup();
#endif

#if HASP_USE_MDNS > 0
    mdnsSetup();
#endif

#if HASP_USE_OTA > 0
    otaSetup();
#endif

#if HASP_USE_HTTP > 0
    httpSetup();
#endif

#if HASP_USE_CONSOLE > 0
    consoleSetup();
#endif

#if HASP_USE_TELNET > 0
    telnetSetup();
#endif

#if HASP_USE_TASMOTA_CLIENT > 0
    slaveSetup();
#endif

    mainLastLoopTime = -1000; // reset loop counter
    delay(20);
    // guiStart();
}

IRAM_ATTR void loop()
{
    guiLoop();
    // haspLoop();

    networkLoop();

#if HASP_USE_GPIO > 0
    gpioLoop();
#endif // GPIO

#if HASP_USE_MQTT > 0
    mqttLoop();
#endif // MQTT

    // haspDevice.loop();

#if HASP_USE_CONSOLE > 0
    // debugLoop();
    consoleLoop();
#endif

    statLoopCounter++;
    /* Timer Loop */
    if(millis() - mainLastLoopTime >= 1000) {
        mainLastLoopTime += 1000;

        /* Runs Every Second */
        haspEverySecond(); // sleep timer & statusupdate

#if HASP_USE_TELNET > 0
        telnetEverySecond();
#endif

        // debugEverySecond();

        switch(++mainLoopCounter) {
            case 1:
                haspDevice.loop_5s();
                break;

            case 2:
#if HASP_USE_HTTP > 0
                // httpEvery5Seconds();
#endif
                break;

            case 3:
#if HASP_USE_GPIO > 0
                //   gpioEvery5Seconds();
#endif
                break;

            case 4:
                isConnected = networkEvery5Seconds(); // Check connection

#if HASP_USE_MQTT > 0
                mqttEvery5Seconds(isConnected);
#endif
                break;

            case 5:
                mainLoopCounter = 0;
                if(statLoopCounter)
                    LOG_VERBOSE(TAG_MAIN, F("%d millis per loop, %d counted"), 5000 / statLoopCounter, statLoopCounter);
                statLoopCounter = 0;
                break;
        }
    }

#ifdef ARDUINO_ARCH_ESP8266
    delay(2); // ms
#else
    delay(2); // ms
              // delay((lv_task_get_idle() >> 5) + 3); // 2..5 ms
#endif
}

#endif

/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifdef WINDOWS

    #include "lvgl.h"
    #include "app_hal.h"

    #include "hasp_conf.h"
    #include "hasp_debug.h"

    #include "hasp/hasp_dispatch.h"
    #include "hasp/hasp.h"

    #include "dev/device.h"
    #include "app_hal.h"

bool isConnected;
uint8_t mainLoopCounter        = 0;
unsigned long mainLastLoopTime = 0;

void debugLvglLogEvent(lv_log_level_t level, const char * file, uint32_t line, const char * funcname,
                       const char * descr)
{
    printf("%s %d\n", file, line);
}

void setup()
{
    printf("%s %d\n", __FILE__, __LINE__);
    fflush(stdout);
    lv_init();
    lv_log_register_print_cb(debugLvglLogEvent);

    printf("%s %d\n", __FILE__, __LINE__);
    fflush(stdout);
    hal_setup();

    printf("%s %d\n", __FILE__, __LINE__);
    haspDevice.pre_setup();

    printf("%s %d\n", __FILE__, __LINE__);
    dispatchSetup();
    //    debugSetup(); // Init the console

    #if HASP_USE_MQTT > 0
    printf("%s %d\n", __FILE__, __LINE__);
    mqttSetup(); // Load Hostname before starting WiFi
    #endif

    printf("%s %d\n", __FILE__, __LINE__);
    haspSetup();
    mainLastLoopTime = millis() - 1000; // reset loop counter
    delay(250);

    mqttStart();
}

void loop()
{
    haspLoop();

    //    debugLoop(); // Console
    haspDevice.loop();

    /* Timer Loop */
    if(millis() - mainLastLoopTime >= 1000) {
        /* Runs Every Second */
        haspEverySecond(); // sleep timer

    #if HASP_USE_OTA > 0
        otaEverySecond(); // progressbar
    #endif

        /* Runs Every 5 Seconds */
        if(mainLoopCounter == 0 || mainLoopCounter == 5) {

            haspDevice.loop_5s();
        }

        /* Reset loop counter every 10 seconds */
        if(mainLoopCounter >= 9) {
            mainLoopCounter = 0;
        } else {
            mainLoopCounter++;
        }
        mainLastLoopTime += 1000;
    }

    delay(6);
}

    #ifdef WINDOWS
int main(int argv, char ** args)
{
    printf("%s %d\n", __FILE__, __LINE__);
    fflush(stdout);
    setup();
    std::cout << "HSetup OK\n";

    while(1) {
        SDL_Delay(5);
        lv_task_handler();
        fflush(stdout);
    }
    std::cout << "Hloop OK\n";

    return 0;
}

    #else
void loop()
{
    delay(5);
    lv_task_handler();
}

    #endif

#endif
/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
For full license information read the LICENSE file in the project folder */

#include "hasplib.h"
#include "hasp_task.h"
#include "sys/net/hasp_network.h"
#include "dev/device.h"

#if HASP_USE_CONFIG > 0
#include "hasp_debug.h"
#include "hasp_macro.h"
#endif

#if HASP_USE_CONFIG > 0
#include "hasp_gui.h"
#endif

#ifdef HASP_USE_STAT_COUNTER
extern uint16_t statLoopCounter; // measures the average looptime
#endif

/* Runs Every Second */
void task_every_second_cb(lv_task_t* task)
{
    haspEverySecond(); // sleep timer & statusupdate

#if HASP_MQTT_TELNET > 0
    mqttEverySecond();
#endif

#if HASP_USE_TELNET > 0
    telnetEverySecond();
#endif

#if defined(HASP_USE_CUSTOM) && HASP_USE_CUSTOM > 0
    custom_every_second();
#endif
    // debugEverySecond();

    switch(task->repeat_count) {
        case 1:
            haspDevice.loop_5s();

            // task is about to get deleted
            if(task->repeat_count == 1) task->repeat_count = 6;

            break;

        case 2:
#if HASP_USE_GPIO > 0
            //   gpioEvery5Seconds();
#endif
            break;

        case 3:
#if defined(HASP_USE_CUSTOM) && HASP_USE_CUSTOM > 0
            custom_every_5seconds();
#endif
            break;

        case 4: {
#ifdef ARDUINO
            bool isConnected = networkEvery5Seconds(); // Check connection

#if HASP_USE_MQTT > 0
            mqttEvery5Seconds(isConnected);
#endif
#endif // ARDUINO
            break;
        }

        case 5:
#ifdef HASP_USE_STAT_COUNTER
            if(statLoopCounter)
                LOG_DEBUG(TAG_MAIN, F("%d millis per loop, %d counted"), 5000 / statLoopCounter, statLoopCounter);
            statLoopCounter = 0;
#endif
            break;
    }
}

void task_teleperiod_cb(lv_task_t* task)
{
#if HASP_USE_MQTT > 0
    if(!mqttIsConnected()) return;

    switch(task->repeat_count) {
        case 1:
            dispatch_send_sensordata(NULL, NULL, TAG_MSGR);
            break;
        case 2:
            dispatch_send_discovery(NULL, NULL, TAG_MSGR);
            break;
        case 3:
            dispatch_statusupdate(NULL, NULL, TAG_MSGR);
            break;
    }

    // task is about to get deleted
    if(task->repeat_count == 1) task->repeat_count = 4;
#endif
}
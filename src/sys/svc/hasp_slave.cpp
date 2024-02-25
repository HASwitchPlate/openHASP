/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "hasplib.h"
#if HASP_USE_TASMOTA_CLIENT > 0

#include "hasp_slave.h"

#include "hasp_gui.h"
#include "hasp_hal.h"
#include "hasp_config.h"
#include "tasmotaSlave.h"

// set RX and TX pins
HardwareSerial Serial2(PD6, PD5);
TasmotaSlave slave(&Serial2);

#define slaveNodeTopic "hasp/"

unsigned long updateLedTimer = 0;    // timer in msec for tele mqtt send
unsigned long updatLedPeriod = 1000; // timer in msec for tele mqtt send

bool ledstate = false;

void slave_send_state(const __FlashStringHelper* subtopic, const char* payload)
{
    // page = 0
    // p[0].b[0].attr = abc
    // dim = 100
    // idle = 0/1
    // light = 0/1
    // brightness = 100

    char cBuffer[strlen(payload) + 64];
    memset(cBuffer, 0, sizeof(cBuffer));
    snprintf_P(cBuffer, sizeof(cBuffer), PSTR("publish %sstate/%s %s"), slaveNodeTopic, subtopic, payload);
    slave.ExecuteCommand((char*)cBuffer);

    // Log after char buffers are cleared
    LOG_TRACE(TAG_TASM, F("TAS PUB: %sstate/%S = %s"), slaveNodeTopic, subtopic, payload);
}

void slave_send_obj_attribute_str(uint8_t pageid, uint8_t btnid, const char* attribute, const char* data)
{
    char cBuffer[192];
    memset(cBuffer, 0, sizeof(cBuffer));
    snprintf_P(cBuffer, sizeof(cBuffer), PSTR("publish %sstate/json {\"p[%u].b[%u].%s\":\"%s\"}"), slaveNodeTopic,
               pageid, btnid, attribute, data);
    slave.ExecuteCommand((char*)cBuffer);
    // Log after char buffers are cleared
    LOG_TRACE(TAG_TASM, F("TAS PUB: %sstate/json = {\"p[%u].b[%u].%s\":\"%s\"}"), slaveNodeTopic, pageid, btnid,
              attribute, data);
}

void slave_send_input(uint8_t id, const char* payload)
{
    // LOG_VERBOSE(TAG_TASM,F("MQTT TST: %sstate/input%u = %s"), mqttNodeTopic, id, payload); // to be removed

    char cBuffer[strlen(payload) + 64];
    memset(cBuffer, 0, sizeof(cBuffer));
    snprintf_P(cBuffer, sizeof(cBuffer), PSTR("publish %sstate/input%u %s"), slaveNodeTopic, id, payload);
    slave.ExecuteCommand((char*)cBuffer);

    // Log after char buffers are cleared
    LOG_TRACE(TAG_TASM, F("TAS PUB: %sstate/input%u = %s"), slaveNodeTopic, id, payload);
}

void TASMO_TELE_JSON()
{ // Periodically publish a JSON string indicating system status
    char data[3 * 128];
    {
        char buffer[128];

        snprintf_P(data, sizeof(data), PSTR("{\"status\":\"available\",\"version\":\"%s\",\"uptime\":%lu,"),
                   haspDevice.get_version(), long(millis() / 1000));

        snprintf_P(buffer, sizeof(buffer), PSTR("\"espCanUpdate\":\"false\",\"page\":%u,\"numPages\":%u,"),
                   haspPages.get(), (HASP_NUM_PAGES));
        strcat(data, buffer);
        snprintf_P(buffer, sizeof(buffer), PSTR("\"tftDriver\":\"%s\",\"tftWidth\":%u,\"tftHeight\":%u}"),
                   halDisplayDriverName().c_str(), (TFT_WIDTH), (TFT_HEIGHT));
        strcat(data, buffer);
    }
    slave.sendJSON((char*)data);
    // slave_send_state(F("statusupdate"), data);
    // debugLastMillis = millis();
}

void TASMO_DATA_RECEIVE(char* data)
{
    LOG_INFO(TAG_TASM, F("Slave IN [%s]"), data);

    char dataType[3];
    memset(dataType, 0, sizeof(dataType));
    snprintf_P(dataType, sizeof(dataType), data);
    LOG_INFO(TAG_TASM, F("dataType [%s]"), dataType);

    if(!strcmp(dataType, "p[")) { //
        dispatchTextLine(data);
    } else if(!strcmp(dataType, "[\"")) {
        dispatchJson(data);
    } else {
        char slvCmd[20], slvVal[60];
        memset(slvCmd, 0, sizeof(slvCmd));
        memset(slvVal, 0, sizeof(slvVal));
        sscanf(data, "%[^=] =%s", slvCmd, slvVal);

        LOG_INFO(TAG_TASM, F("Cmd[%s] Val[%s]"), slvCmd, slvVal);

        if(!strcmp(slvCmd, "calData")) {
            if(strlen(slvVal) != 0) {
                char cBuffer[strlen(slvVal) + 24];
                memset(cBuffer, 0, sizeof(cBuffer));
                snprintf_P(cBuffer, sizeof(cBuffer), PSTR("{'calibration':[%s]}"), slvVal);
                dispatchConfig("gui", cBuffer);
            } else {
                dispatchConfig("gui", "");
            }
        } else if(!strcmp(slvCmd, "jsonl")) {
            dispatchJsonl(slvVal);
        } else if(!strcmp(slvCmd, "clearpage")) {
            dispatchClearPage(slvVal);
        } else {
            dispatchTextLine(data);
        }
    }
}

void TASMO_EVERY_SECOND(void)
{
    if(ledstate) {
        ledstate = false;
        // digitalWrite(HASP_OUTPUT_PIN, 1);
        // LOG_INFO(TAG_TASM,F("LED OFF"));
    } else {
        ledstate = true;
        // digitalWrite(HASP_OUTPUT_PIN, 0);
        // LOG_INFO(TAG_TASM,F("LED ON"));
    }
}

void slaveSetup()
{
    Serial2.begin(HASP_TASMOTACLIENT_SPEED);
    // slave.attach_FUNC_EVERY_SECOND(TASMO_EVERY_SECOND);
    slave.attach_FUNC_JSON(TASMO_TELE_JSON);
    slave.attach_FUNC_COMMAND_SEND(TASMO_DATA_RECEIVE);

    LOG_TRACE(TAG_TASM, F("HASP SLAVE LOADED"));
}

void slaveLoop(void)
{
    slave.loop();
    // demo code to run the led without tasmota
    // if ((millis() - updateLedTimer) >= updatLedPeriod) {
    //     updateLedTimer = millis();
    //     TASMO_EVERY_SECOND();
    // }
}

#endif
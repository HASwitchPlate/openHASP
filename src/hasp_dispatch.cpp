/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "ArduinoJson.h"
#include "ArduinoLog.h"
#include "StringStream.h"
#include "CharStream.h"

#include "hasp_conf.h"

#include "hasp_dispatch.h"
#include "hasp_config.h"
#include "hasp_debug.h"
#include "hasp_object.h"
#include "hasp_gui.h"
#include "hasp_oobe.h"
#include "hasp_hal.h"
#include "hasp.h"

uint8_t nCommands = 0;
haspCommand_t commands[15];

bool is_true(const char * s)
{
    return (!strcasecmp_P(s, PSTR("true")) || !strcasecmp_P(s, PSTR("on")) || !strcasecmp_P(s, PSTR("yes")) ||
            !strcmp_P(s, PSTR("1")));
}

// Format filesystem and erase EEPROM
bool dispatch_factory_reset()
{
    bool formated, erased = true;

#if HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0
    formated = HASP_FS.format();
#endif

#if HASP_USE_EEPROM > 0
    erased = false;
#endif

    return formated && erased;
}

void dispatchGpioOutput(int output, bool state)
{
    int pin = 0;

    if(pin >= 0) {
        Log.notice(TAG_MSGR, F("PIN OUTPUT STATE %d"), state);

#if defined(ARDUINO_ARCH_ESP32)
        ledcWrite(99, state ? 1023 : 0); // ledChannel and value
#elif defined(STM32F4xx)
        digitalWrite(HASP_OUTPUT_PIN, state);
#else
        digitalWrite(D1, state);
        // analogWrite(pin, state ? 1023 : 0);
#endif
    }
}

void dispatchGpioOutput(String strTopic, const char * payload)
{
    String strTemp((char *)0);
    strTemp.reserve(128);
    strTemp = strTopic.substring(7, strTopic.length());
    dispatchGpioOutput(strTemp.toInt(), is_true(payload));
}

// p[x].b[y]=value
inline void dispatch_process_button_attribute(String strTopic, const char * payload)
{
    // Log.verbose(TAG_MSGR,F("BTN ATTR: %s = %s"), strTopic.c_str(), payload);

    String strPageId((char *)0);
    String strTemp((char *)0);

    strPageId = strTopic.substring(2, strTopic.indexOf("]"));
    strTemp   = strTopic.substring(strTopic.indexOf("]") + 1, strTopic.length());

    if(strTemp.startsWith(".b[")) {
        String strObjId((char *)0);
        String strAttr((char *)0);

        strObjId = strTemp.substring(3, strTemp.indexOf("]"));
        strAttr  = strTemp.substring(strTemp.indexOf("]") + 1, strTemp.length());
        // debugPrintln(strPageId + " && " + strObjId + " && " + strAttr);

        int pageid = strPageId.toInt();
        int objid  = strObjId.toInt();

        if(pageid >= 0 && pageid <= 255 && objid >= 0 && objid <= 255) {
            hasp_process_attribute((uint8_t)pageid, (uint8_t)objid, strAttr.c_str(), payload);
        } // valid page
    }
}

// objectattribute=value
void dispatchCommand(const char * topic, const char * payload)
{
    /* ================================= Standard payload commands ======================================= */

    // check and execute commands from commands array
    for(int i = 0; i < nCommands; i++) {
        if(!strcasecmp_P(topic, commands[i].p_cmdstr)) {
            // Log.warning(TAG_MSGR, F("Command %d found in array !!!"), i);
            commands[i].func((char *)payload); /* execute command */
            return;
        }
    }

    /* =============================== Not standard payload commands ===================================== */

    if(strlen(topic) == 7 && topic == strstr_P(topic, PSTR("output"))) {
        dispatchGpioOutput(topic, payload);

    } else if(strcasecmp_P(topic, PSTR("screenshot")) == 0) {
        guiTakeScreenshot("/screenshot.bmp"); // Literal String

    } else if(topic == strstr_P(topic, PSTR("p["))) {
        dispatch_process_button_attribute(topic, payload);

#if HASP_USE_WIFI > 0
    } else if(!strcmp_P(topic, F_CONFIG_SSID) || !strcmp_P(topic, F_CONFIG_PASS)) {
        DynamicJsonDocument settings(45);
        settings[topic] = payload;
        wifiSetConfig(settings.as<JsonObject>());
#endif

    } else if(!strcmp_P(topic, PSTR("mqtthost")) || !strcmp_P(topic, PSTR("mqttport")) ||
              !strcmp_P(topic, PSTR("mqttport")) || !strcmp_P(topic, PSTR("mqttuser")) ||
              !strcmp_P(topic, PSTR("hostname"))) {
        // char item[5];
        // memset(item, 0, sizeof(item));
        // strncpy(item, topic + 4, 4);
        DynamicJsonDocument settings(45);
        settings[topic + 4] = payload;
        mqttSetConfig(settings.as<JsonObject>());

    } else {
        if(strlen(payload) == 0) {
            //    dispatchTextLine(topic); // Could cause an infinite loop!
        }
        Log.warning(TAG_MSGR, F("Command '%s' not found => %s"), topic, payload);
    }
}

// Strip command/config prefix from the topic and process the payload
void dispatchTopicPayload(const char * topic, const char * payload)
{
    // Log.verbose(TAG_MSGR,F("TOPIC: short topic: %s"), topic);

    if(!strcmp_P(topic, PSTR("command"))) {
        dispatchTextLine((char *)payload);
        return;
    }

    if(topic == strstr_P(topic, PSTR("command/"))) { // startsWith command/
        topic += 8u;
        // Log.verbose(TAG_MSGR,F("MQTT IN: command subtopic: %s"), topic);

        // '[...]/device/command/p[1].b[4].txt' -m '"Lights On"' ==
        // nextionSetAttr("p[1].b[4].txt", "\"Lights On\"")
        dispatchCommand(topic, (char *)payload);

        return;
    }

    if(topic == strstr_P(topic, PSTR("config/"))) { // startsWith command/
        topic += 7u;
        dispatchConfig(topic, (char *)payload);
        return;
    }

    dispatchCommand(topic, (char *)payload); // dispatch as is
}

// Parse one line of text and execute the command(s)
void dispatchTextLine(const char * cmnd)
{
    // dispatchPrintln(F("CMND"), cmnd);

    if(cmnd == strstr_P(cmnd, PSTR("page "))) { // startsWith command/
        dispatchPage(cmnd + 5);

    } else {
        size_t pos1 = std::string(cmnd).find("=");
        size_t pos2 = std::string(cmnd).find(" ");
        int pos     = 0;

        if(pos1 != std::string::npos) {
            if(pos2 != std::string::npos) {
                pos = (pos1 < pos2 ? pos1 : pos2);
            } else {
                pos = pos1;
            }

        } else if(pos2 != std::string::npos) {
            pos = pos2;
        } else {
            pos = 0;
        }

        if(pos > 0) {
            String strTopic((char *)0);
            // String strPayload((char *)0);

            strTopic.reserve(pos + 1);
            // strPayload.reserve(128);

            strTopic = String(cmnd).substring(0, pos);
            // strPayload = cmnd.substring(pos + 1, cmnd.length());
            // cmnd[pos] = 0; // change '=' character into '\0'

            dispatchTopicPayload(strTopic.c_str(),
                                 cmnd + pos + 1); // topic is before '=', payload is after '=' position
        } else {
            char buf[1] = {0};
            dispatchTopicPayload(cmnd, buf);
        }
    }
}

// send idle state to the client
void dispatch_output_idle_state(const char * state)
{
#if !defined(HASP_USE_MQTT) && !defined(HASP_USE_TASMOTA_SLAVE)
    Log.notice(TAG_MSGR, F("idle = %s"), state);
#else

#if HASP_USE_MQTT > 0
    mqtt_send_state(F("idle"), state);
#endif

#if HASP_USE_TASMOTA_SLAVE > 0
    slave_send_state(F("idle"), state);
#endif

#endif
}

// restart the device
void dispatchReboot(bool saveConfig)
{
    if(saveConfig) configWriteConfig();
#if HASP_USE_MQTT > 0
    mqttStop(); // Stop the MQTT Client first
#endif
    debugStop();
#if HASP_USE_WIFI > 0
    wifiStop();
#endif
    Log.verbose(TAG_MSGR, F("-------------------------------------"));
    Log.notice(TAG_MSGR, F("HALT: Properly Rebooting the MCU now!"));
    Serial.flush();
    halRestartMcu();
}

void dispatch_button(uint8_t id, const char * event)
{
#if !defined(HASP_USE_MQTT) && !defined(HASP_USE_TASMOTA_SLAVE)
    Log.notice(TAG_MSGR, F("input%d = %s"), id, event);
#else
#if HASP_USE_MQTT > 0
    mqtt_send_input(id, event);
#endif
#if HASP_USE_TASMOTA_SLAVE > 0
    slave_send_input(id, event);
#endif
#endif
}

// Map events to either ON or OFF (UP or DOWN)
bool dispatch_get_event_state(uint8_t eventid)
{
    switch(eventid) {
        case HASP_EVENT_ON:
        case HASP_EVENT_DOWN:
        case HASP_EVENT_LONG:
        case HASP_EVENT_HOLD:
            return true;
        case HASP_EVENT_OFF:
        case HASP_EVENT_UP:
        case HASP_EVENT_SHORT:
        case HASP_EVENT_DOUBLE:
        case HASP_EVENT_LOST:
        default:
            return false;
    }
}

// Map events to their description string
void dispatch_get_event_name(uint8_t eventid, char * buffer, size_t size)
{
    switch(eventid) {
        case HASP_EVENT_ON:
            memcpy_P(buffer, PSTR("ON"), size);
            break;
        case HASP_EVENT_OFF:
            memcpy_P(buffer, PSTR("OFF"), size);
            break;
        case HASP_EVENT_UP:
            memcpy_P(buffer, PSTR("UP"), size);
            break;
        case HASP_EVENT_DOWN:
            memcpy_P(buffer, PSTR("DOWN"), size);
            break;
        case HASP_EVENT_SHORT:
            memcpy_P(buffer, PSTR("SHORT"), size);
            break;
        case HASP_EVENT_LONG:
            memcpy_P(buffer, PSTR("LONG"), size);
            break;
        case HASP_EVENT_HOLD:
            memcpy_P(buffer, PSTR("HOLD"), size);
            break;
        case HASP_EVENT_LOST:
            memcpy_P(buffer, PSTR("LOST"), size);
            break;
        default:
            memcpy_P(buffer, PSTR("UNKNOWN"), size);
    }
}

void dispatch_send_group_event(uint8_t groupid, uint8_t eventid, bool update_hasp)
{
    // update outputs
    gpio_set_group_outputs(groupid, eventid);

    // send out value
#if !defined(HASP_USE_MQTT) && !defined(HASP_USE_TASMOTA_SLAVE)
    Log.notice(TAG_MSGR, F("group%d = %s"), groupid, eventid);
#else
#if HASP_USE_MQTT > 0
    // mqtt_send_input(id, event);
#endif
#if HASP_USE_TASMOTA_SLAVE > 0
    // slave_send_input(id, event);
#endif
#endif

    // update objects, except src_obj
    if(update_hasp) hasp_set_group_objects(groupid, eventid, NULL);
}

void IRAM_ATTR dispatch_send_obj_attribute_str(uint8_t pageid, uint8_t btnid, const char * attribute, const char * data)
{
#if !defined(HASP_USE_MQTT) && !defined(HASP_USE_TASMOTA_SLAVE)
    Log.notice(TAG_MSGR, F("json = {\"p[%u].b[%u].%s\":\"%s\"}"), pageid, btnid, attribute, data);
#else
#if HASP_USE_MQTT > 0
    mqtt_send_obj_attribute_str(pageid, btnid, attribute, data);
#endif
#if HASP_USE_TASMOTA_SLAVE > 0
    slave_send_obj_attribute_str(pageid, btnid, attribute, data);
#endif
#endif
}

void dispatch_send_object_event(uint8_t pageid, uint8_t objid, uint8_t eventid)
{
    if(objid < 100) {
        char topic[8];
        char payload[8];
        snprintf_P(topic, sizeof(topic), PSTR("event"));
        dispatch_get_event_name(eventid, payload, sizeof(payload));
        dispatch_send_obj_attribute_str(pageid, objid, topic, payload);
    } else {
        uint8_t groupid = (objid - 100) / 10;
        dispatch_send_group_event(groupid, eventid, true);
    }
}

// send return output back to the client
void IRAM_ATTR dispatch_obj_attribute_str(uint8_t pageid, uint8_t btnid, const char * attribute, const char * data)
{
#if !defined(HASP_USE_MQTT) && !defined(HASP_USE_TASMOTA_SLAVE)
    Log.notice(TAG_MSGR, F("json = {\"p[%u].b[%u].%s\":\"%s\"}"), pageid, btnid, attribute, data);
#else
#if HASP_USE_MQTT > 0
    mqtt_send_obj_attribute_str(pageid, btnid, attribute, data);
#endif
#if HASP_USE_TASMOTA_SLAVE > 0
    slave_send_obj_attribute_str(pageid, btnid, attribute, data);
#endif
#endif
}

// Get or Set a part of the config.json file
void dispatchConfig(const char * topic, const char * payload)
{
    DynamicJsonDocument doc(128 * 2);
    char buffer[128 * 2];
    JsonObject settings;
    bool update;

    if(strlen(payload) == 0) {
        // Make sure we have a valid JsonObject to start from
        settings = doc.to<JsonObject>().createNestedObject(topic);
        update   = false;

    } else {
        DeserializationError jsonError = deserializeJson(doc, payload);
        if(jsonError) { // Couldn't parse incoming JSON command
            Log.warning(TAG_MSGR, F("JSON: Failed to parse incoming JSON command with error: %s"), jsonError.c_str());
            return;
        }
        settings = doc.as<JsonObject>();
        update   = true;
    }

    if(strcasecmp_P(topic, PSTR("debug")) == 0) {
        if(update)
            debugSetConfig(settings);
        else
            debugGetConfig(settings);
    }

    else if(strcasecmp_P(topic, PSTR("gui")) == 0) {
        if(update)
            guiSetConfig(settings);
        else
            guiGetConfig(settings);
    }

    else if(strcasecmp_P(topic, PSTR("hasp")) == 0) {
        if(update)
            haspSetConfig(settings);
        else
            haspGetConfig(settings);
    }

#if HASP_USE_WIFI > 0
    else if(strcasecmp_P(topic, PSTR("wifi")) == 0) {
        if(update)
            wifiSetConfig(settings);
        else
            wifiGetConfig(settings);
    }
#if HASP_USE_MQTT > 0
    else if(strcasecmp_P(topic, PSTR("mqtt")) == 0) {
        if(update)
            mqttSetConfig(settings);
        else
            mqttGetConfig(settings);
    }
#endif
#if HASP_USE_TELNET > 0
    //   else if(strcasecmp_P(topic, PSTR("telnet")) == 0)
    //       telnetGetConfig(settings[F("telnet")]);
#endif
#if HASP_USE_MDNS > 0
    else if(strcasecmp_P(topic, PSTR("mdns")) == 0) {
        if(update)
            mdnsSetConfig(settings);
        else
            mdnsGetConfig(settings);
    }
#endif
#if HASP_USE_HTTP > 0
    else if(strcasecmp_P(topic, PSTR("http")) == 0) {
        if(update)
            httpSetConfig(settings);
        else
            httpGetConfig(settings);
    }
#endif
#endif

    // Send output
    if(!update) {
        settings.remove(F("pass")); // hide password in output
        size_t size = serializeJson(doc, buffer, sizeof(buffer));
#if !defined(HASP_USE_MQTT) && !defined(HASP_USE_TASMOTA_SLAVE)
        Log.notice(TAG_MSGR, F("config %s = %s"), topic, buffer);
#else
#if HASP_USE_MQTT > 0
        mqtt_send_state(F("config"), buffer);
#endif
#if HASP_USE_TASMOTA > 0
        slave_send_state(F("config"), buffer);
#endif
#endif
    }
}

/********************************************** Native Commands ****************************************/

void dispatchParseJson(const char * payload)
{ // Parse an incoming JSON array into individual commands
    /*  if(strPayload.endsWith(",]")) {
          // Trailing null array elements are an artifact of older Home Assistant automations
          // and need to be removed before parsing by ArduinoJSON 6+
          strPayload.remove(strPayload.length() - 2, 2);
          strPayload.concat("]");
      }*/
    size_t maxsize = (128u * ((strlen(payload) / 128) + 1)) + 256;
    DynamicJsonDocument json(maxsize);
    DeserializationError jsonError = deserializeJson(json, (char *)payload);
    // json.shrinkToFit();

    if(jsonError) { // Couldn't parse incoming JSON command
        Log.warning(TAG_MSGR, F("JSON: Failed to parse incoming JSON command with error: %s"), jsonError.c_str());
    } else {

        if(json.is<JsonArray>()) {
            // handle json as an array of commands
            JsonArray arr = json.as<JsonArray>();
            for(JsonVariant command : arr) {
                dispatchTextLine(command.as<String>().c_str());
            }
        } else if(json.is<JsonObject>()) {
            // handle json as a jsonl
            uint8_t savedPage = haspGetPage();
            hasp_new_object(json.as<JsonObject>(), savedPage);
        } else {
            // handle json as a single command
            dispatchTextLine(json.as<const char *>());
        }
    }
}

void dispatchParseJsonl(Stream & stream)
{
    DynamicJsonDocument jsonl(4 * 128u);
    uint8_t savedPage = haspGetPage();

    // Log.notice(TAG_MSGR,F("DISPATCH: jsonl"));

    while(deserializeJson(jsonl, stream) == DeserializationError::Ok) {
        // serializeJson(jsonl, Serial);
        // Serial.println();
        hasp_new_object(jsonl.as<JsonObject>(), savedPage);
    }
}

void dispatchParseJsonl(const char * payload)
{
    CharStream stream((char *)payload);
    dispatchParseJsonl(stream);
}

void dispatchCurrentPage()
{
    // Log result
    char buffer[4];
    itoa(haspGetPage(), buffer, DEC);

#if HASP_USE_MQTT > 0
    mqtt_send_state(F("page"), buffer);
#endif

#if HASP_USE_TASMOTA_SLAVE > 0
    slave_send_state(F("page"), buffer);
#endif
}

// Get or Set a page
void dispatchPage(const char * page)
{
    if(strlen(page) > 0 && atoi(page) < HASP_NUM_PAGES) {
        haspSetPage(atoi(page));
    }

    dispatchCurrentPage();
}

void dispatchPageNext()
{
    uint8_t page = haspGetPage();
    if(page + 1 >= HASP_NUM_PAGES) {
        page = 0;
    } else {
        page++;
    }
    haspSetPage(page);
    dispatchCurrentPage();
}

void dispatchPagePrev()
{
    uint8_t page = haspGetPage();
    if(page == 0) {
        page = HASP_NUM_PAGES - 1;
    } else {
        page--;
    }
    haspSetPage(page);
    dispatchCurrentPage();
}

// Clears a page id or the current page if empty
void dispatchClearPage(const char * page)
{
    if(strlen(page) == 0) {
        haspClearPage(haspGetPage());
    } else {
        haspClearPage(atoi(page));
    }
}

void dispatchDim(const char * level)
{
    // Set the current state
    if(strlen(level) != 0) guiSetDim(atoi(level));
    //  dispatchPrintln(F("DIM"), strDimLevel);
    char buffer[4];

#if defined(HASP_USE_MQTT) || defined(HASP_USE_TASMOTA_SLAVE)
    itoa(guiGetDim(), buffer, DEC);

#if HASP_USE_MQTT > 0
    mqtt_send_state(F("dim"), buffer);
#endif

#if HASP_USE_TASMOTA_SLAVE > 0
    slave_send_state(F("dim"), buffer);
#endif

#endif
}

void dispatchBacklight(const char * payload)
{
    // strPayload.toUpperCase();
    // dispatchPrintln(F("LIGHT"), strPayload);

    // Set the current state
    if(strlen(payload) != 0) guiSetBacklight(is_true(payload));

    // Return the current state
    char buffer[4];
    memcpy_P(buffer, guiGetBacklight() ? PSTR("ON") : PSTR("OFF"), sizeof(buffer));

#if HASP_USE_MQTT > 0
    mqtt_send_state(F("light"), buffer);
#endif

#if HASP_USE_TASMOTA_SLAVE > 0
    slave_send_state(F("light"), buffer);
#endif
}

// Send status update message to the client
void dispatch_output_statusupdate()
{
#if HASP_USE_MQTT > 0
    mqtt_send_statusupdate();
#endif
}

void dispatchWebUpdate(const char * espOtaUrl)
{
#if HASP_USE_OTA > 0
    Log.notice(TAG_MSGR, F("Checking for updates at URL: %s"), espOtaUrl);
    otaHttpUpdate(espOtaUrl);
#endif
}

/******************************************* Command Wrapper Functions *********************************/

void dispatch_output_statusupdate(const char *)
{
    dispatch_output_statusupdate();
}

void dispatchCalibrate(const char *)
{
    guiCalibrate();
}

void dispatchWakeup(const char *)
{
    haspWakeUp();
}

void dispatchReboot(const char *)
{
    dispatchReboot(true);
}

void dispatch_factory_reset(const char *)
{
    dispatch_factory_reset();
    delay(250);
    dispatchReboot(false); // don't save config
}

/******************************************* Commands builder *******************************************/

static void dispatch_add_command(const char * p_cmdstr, void (*func)(const char *))
{
    if(nCommands >= sizeof(commands) / sizeof(haspCommand_t)) {
        Log.fatal(TAG_MSGR, F("Dispatchcer command array overflow: %d"), nCommands);
        while(1) {
        }
    } else {
        commands[nCommands].p_cmdstr = p_cmdstr;
        commands[nCommands].func     = func;
        nCommands++;
    }
}

void dispatchSetup()
{
    // In order of importance : commands are NOT case-sensitive
    // The command.func() call will receive the full payload as ONLY parameter!

    /* WARNING: remember to expand the commands array when adding new commands */

    dispatch_add_command(PSTR("json"), dispatchParseJson);
    dispatch_add_command(PSTR("page"), dispatchPage);
    dispatch_add_command(PSTR("wakeup"), dispatchWakeup);
    dispatch_add_command(PSTR("statusupdate"), dispatch_output_statusupdate);
    dispatch_add_command(PSTR("clearpage"), dispatchClearPage);
    dispatch_add_command(PSTR("jsonl"), dispatchParseJsonl);
    dispatch_add_command(PSTR("dim"), dispatchDim);
    dispatch_add_command(PSTR("brightness"), dispatchDim);
    dispatch_add_command(PSTR("light"), dispatchBacklight);
    dispatch_add_command(PSTR("calibrate"), dispatchCalibrate);
    dispatch_add_command(PSTR("update"), dispatchWebUpdate);
    dispatch_add_command(PSTR("reboot"), dispatchReboot);
    dispatch_add_command(PSTR("restart"), dispatchReboot);
    dispatch_add_command(PSTR("factoryreset"), dispatch_factory_reset);
    dispatch_add_command(PSTR("setupap"), oobeFakeSetup);
}

void IRAM_ATTR dispatchLoop()
{
    // Not used
}

/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include <stdint.h>

//#include "ArduinoLog.h"
#include "hasplib.h"

#include "dev/device.h"
#include "drv/tft_driver.h"

//#include "hasp_gui.h"

#if HASP_USE_DEBUG > 0
#include "../hasp_debug.h"
#include "hasp_gui.h" // for screenshot

#if defined(WINDOWS) || defined(POSIX)
#include <iostream>
#include <fstream>
#include <sstream>
#include "../mqtt/hasp_mqtt.h"
#else
#include "StringStream.h"
#include "CharStream.h"

#include "hasp_oobe.h"
#include "sys/gpio/hasp_gpio.h"
#include "hal/hasp_hal.h"

#include "sys/svc/hasp_ota.h"
#include "mqtt/hasp_mqtt.h"
#include "sys/net/hasp_network.h" // for network_get_status()
#endif
#endif

#if HASP_USE_CONFIG > 0
#include "hasp_config.h"
#endif

extern uint8_t hasp_sleep_state;

dispatch_conf_t dispatch_setings = {.teleperiod = 300};

uint32_t dispatchLastMillis = -3000000; // force discovery
uint8_t nCommands           = 0;
haspCommand_t commands[18];

struct moodlight_t
{
    uint8_t power;
    uint8_t r, g, b;
};
moodlight_t moodlight;

static void dispatch_config(const char* topic, const char* payload);
// void dispatch_group_value(uint8_t groupid, int16_t state, lv_obj_t * obj);

/* Sends the payload out on the state/subtopic
 */
void dispatch_state_subtopic(const char* subtopic, const char* payload)
{
#if !defined(HASP_USE_MQTT) && !defined(HASP_USE_TASMOTA_CLIENT)
    LOG_TRACE(TAG_MSGR, F("%s => %s"), subtopic, payload);
#else

#if HASP_USE_MQTT > 0
    switch(mqtt_send_state(subtopic, payload)) {
        case MQTT_ERR_OK:
            LOG_TRACE(TAG_MQTT_PUB, F("%s => %s"), subtopic, payload);
            break;
        case MQTT_ERR_PUB_FAIL:
            LOG_ERROR(TAG_MQTT_PUB, F(D_MQTT_FAILED " %s => %s"), subtopic, payload);
            break;
        case MQTT_ERR_NO_CONN:
            LOG_ERROR(TAG_MQTT, F(D_MQTT_NOT_CONNECTED));
            break;
        default:
            LOG_ERROR(TAG_MQTT, F(D_ERROR_UNKNOWN));
    }
#endif

#if HASP_USE_TASMOTA_CLIENT > 0
    slave_send_state(subtopic, payload);
#endif

#endif
}

// Format filesystem and erase EEPROM
bool dispatch_factory_reset()
{
    bool formated = true;
    bool erased   = true;

#if HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0
    formated = HASP_FS.format();
#endif

#if HASP_USE_EEPROM > 0
    erased = false;
#endif

    return formated && erased;
}

void dispatch_json_error(uint8_t tag, DeserializationError& jsonError)
{
    LOG_ERROR(tag, F(D_JSON_FAILED " %s"), jsonError.c_str());
}

// p[x].b[y].attr=value
static inline bool dispatch_parse_button_attribute(const char* topic_p, const char* payload)
{
    long num;
    char* pEnd;
    uint8_t pageid, objid;

    if(*topic_p != 'p' && *topic_p != 'P') return false; // obligated p
    topic_p++;

    if(*topic_p == '[') { // optional brackets, TODO: remove
        topic_p++;
        num = strtol(topic_p, &pEnd, DEC);
        if(*pEnd != ']') return false; // obligated closing bracket
        pEnd++;

    } else {
        num = strtol(topic_p, &pEnd, DEC);
    }

    if(num < 0 || num > HASP_NUM_PAGES) return false; // page number must be valid

    pageid  = (uint8_t)num;
    topic_p = pEnd;

    if(*topic_p == '.') topic_p++; // optional separator

    if(*topic_p != 'b' && *topic_p != 'B') return false; // obligated b
    topic_p++;

    if(*topic_p == '[') { // optional brackets, TODO: remove
        topic_p++;
        num = strtol(topic_p, &pEnd, DEC);
        if(*pEnd != ']') return false; // obligated closing bracket
        pEnd++;
    } else {
        num = strtol(topic_p, &pEnd, DEC);
    }

    if(num < 0 || num > 255) return false; // id must be valid
    objid   = (uint8_t)num;
    topic_p = pEnd;

    if(*topic_p != '.') return false; // obligated seperator
    topic_p++;

    hasp_process_attribute(pageid, objid, topic_p, payload);
    return true;

    /*
        if(sscanf(topic_p, HASP_OBJECT_NOTATION ".", &pageid, &objid) == 2) { // Literal String

            // OK, continue below

        } else if(sscanf(topic_p, "p[%u].b[%u].", &pageid, &objid) == 2) { // Literal String

            // TODO: obsolete old syntax p[x].b[y].
            // OK, continue below
            while(*topic_p++ != '.') {
                // strip to '.' character
            }

        } else {
            return false;
        }

        while(*topic_p != '.') {
            if(*topic_p == 0) return false; // strip to '.' character
            topic_p++;
        }

        hasp_process_attribute((uint8_t)pageid, (uint8_t)objid, topic_p, payload);
        return true; */

    // String strPageId((char *)0);
    // String strTemp((char *)0);

    // strPageId = strTopic.substring(2, strTopic.indexOf("]"));
    // strTemp   = strTopic.substring(strTopic.indexOf("]") + 1, strTopic.length());

    // if(strTemp.startsWith(".b[")) {
    //     String strObjId((char *)0);
    //     String strAttr((char *)0);

    //     strObjId = strTemp.substring(3, strTemp.indexOf("]"));
    //     strAttr  = strTemp.substring(strTemp.indexOf("]") + 1, strTemp.length());
    //     // debugPrintln(strPageId + " && " + strObjId + " && " + strAttr);

    //     pageid = strPageId.toInt();
    //     objid  = strObjId.toInt();

    //     if(pageid >= 0 && pageid <= 255 && objid >= 0 && objid <= 255) {
    //         hasp_process_attribute(pageid, objid, strAttr.c_str(), payload);
    //     } // valid page
    // }
}

static void dispatch_gpio(const char* topic, const char* payload)
{
#if HASP_USE_GPIO > 0

    int16_t val;
    uint8_t pin;

    if(topic == strstr_P(topic, PSTR("relay"))) {
        topic += 5;
        val = Parser::is_true(payload);

    } else if(topic == strstr_P(topic, PSTR("led"))) {
        topic += 3;
        val = atoi(payload);

    } else if(topic == strstr_P(topic, PSTR("pwm"))) {
        topic += 3;
        val = atoi(payload);

    } else {
        LOG_WARNING(TAG_MSGR, F("Invalid gpio %s"), topic);
        return;
    }

    if(Parser::is_only_digits(topic)) {
        pin = atoi(topic);
        if(strlen(payload) > 0) {
            gpio_set_value(pin, val);
        } else {
            gpio_get_value(pin);
        }
    } else {
        LOG_WARNING(TAG_MSGR, F("Invalid pin %s"), topic);
    }
#endif
}

// objectattribute=value
void dispatch_command(const char* topic, const char* payload)
{
    /* ================================= Standard payload commands ======================================= */

    // check and execute commands from commands array
    for(int i = 0; i < nCommands; i++) {
        if(!strcasecmp_P(topic, commands[i].p_cmdstr)) {
            // LOG_DEBUG(TAG_MSGR, F("Command %d found in array !!!"), i);
            commands[i].func(topic, payload); /* execute command */
            return;
        }
    }

    /* =============================== Not standard payload commands ===================================== */

    if(topic == strstr_P(topic, PSTR("gpio/"))) {

        dispatch_gpio(topic + 5, payload);

        // } else if(strcasecmp_P(topic, PSTR("screenshot")) == 0) {
        //     guiTakeScreenshot("/screenshot.bmp"); // Literal String

    } else if((topic[0] == 'p' || topic[0] == 'P') && dispatch_parse_button_attribute(topic, payload)) {
        return; // matched pxby.attr

#if HASP_USE_CONFIG > 0

#if HASP_USE_WIFI > 0
    } else if(!strcmp_P(topic, FP_CONFIG_SSID) || !strcmp_P(topic, FP_CONFIG_PASS)) {
        StaticJsonDocument<64> settings;
        settings[topic] = payload;
        wifiSetConfig(settings.as<JsonObject>());
#endif // HASP_USE_WIFI

#if HASP_USE_MQTT > 0
    } else if(!strcmp_P(topic, PSTR("mqtthost")) || !strcmp_P(topic, PSTR("mqttport")) ||
              !strcmp_P(topic, PSTR("mqttuser")) || !strcmp_P(topic, PSTR("mqttpass")) ||
              !strcmp_P(topic, PSTR("hostname"))) {
        // char item[5];
        // memset(item, 0, sizeof(item));
        // strncpy(item, topic + 4, 4);

        StaticJsonDocument<64> settings;
        settings[topic + 4] = payload;
        mqttSetConfig(settings.as<JsonObject>());
#endif // HASP_USE_MQTT

#endif // HASP_USE_CONFIG

    } else {
        if(strlen(payload) == 0) {
            //    dispatch_text_line(topic); // Could cause an infinite loop!
        }
        LOG_WARNING(TAG_MSGR, F(D_DISPATCH_COMMAND_NOT_FOUND " => %s"), topic, payload);
    }
}

// Strip command/config prefix from the topic and process the payload
void dispatch_topic_payload(const char* topic, const char* payload)
{
    // LOG_VERBOSE(TAG_MSGR,F("TOPIC: short topic: %s"), topic);

    if(!strcmp_P(topic, PSTR("command"))) {
        dispatch_text_line((char*)payload);
        return;
    }

    if(topic == strstr_P(topic, PSTR("command/"))) { // startsWith command/
        topic += 8u;
        dispatch_command(topic, (char*)payload);
        return;
    }

#if HASP_USE_CONFIG > 0
    if(topic == strstr_P(topic, PSTR("config/"))) { // startsWith command/
        topic += 7u;
        dispatch_config(topic, (char*)payload);
        return;
    }
#endif

    dispatch_command(topic, (char*)payload); // dispatch as is
}

// Parse one line of text and execute the command
void dispatch_text_line(const char* cmnd)
{
    size_t pos1 = std::string(cmnd).find("=");
    size_t pos2 = std::string(cmnd).find(" ");
    size_t pos  = 0;

    // Find what comes first, ' ' or '='
    if(pos1 != std::string::npos) {
        if(pos2 != std::string::npos) {
            pos = (pos1 < pos2 ? pos1 : pos2);
        } else {
            pos = pos1;
        }

    } else {
        pos = (pos2 != std::string::npos) ? pos2 : 0;
    }

    if(pos > 0) { // ' ' or '=' found
        char topic[64];
        memset(topic, 0, sizeof(topic));
        if(pos < sizeof(topic))
            memcpy(topic, cmnd, pos);
        else
            memcpy(topic, cmnd, sizeof(topic) - 1);

        // topic is before '=', payload is after '=' position
        LOG_TRACE(TAG_MSGR, F("%s=%s"), topic, cmnd + pos + 1);
        dispatch_topic_payload(topic, cmnd + pos + 1);
    } else {
        char empty_payload[1] = {0};
        LOG_TRACE(TAG_MSGR, F("%s=%s"), cmnd, empty_payload);
        dispatch_topic_payload(cmnd, empty_payload);
    }
}

void dispatch_get_idle_state(uint8_t state, char* payload)
{
    switch(state) {
        case HASP_SLEEP_LONG:
            memcpy_P(payload, PSTR("long"), 5);
            break;
        case HASP_SLEEP_SHORT:
            memcpy_P(payload, PSTR("short"), 6);
            break;
        default:
            memcpy_P(payload, PSTR("off"), 4);
    }
}

// send idle state to the client
void dispatch_output_idle_state(uint8_t state)
{
    char topic[6];
    char payload[6];
    memcpy_P(topic, PSTR("idle"), 5);

    dispatch_get_idle_state(state, payload);
    dispatch_state_subtopic(topic, payload);
}

// void dispatch_output_group_state(uint8_t groupid, uint16_t state)
// {
//     char payload[64];
//     char number[16]; // Return the current state
//     char topic[8];
//     itoa(state, number, DEC);
//     snprintf_P(payload, sizeof(payload), PSTR("{\"group\":%d,\"state\":\"%s\"}"), groupid, number);

//     memcpy_P(topic, PSTR("output"), 7);
//     dispatch_state_subtopic(topic, payload);
// }

#if HASP_USE_CONFIG > 0
// Get or Set a part of the config.json file
static void dispatch_config(const char* topic, const char* payload)
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
            dispatch_json_error(TAG_MSGR, jsonError);
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

#if HASP_USE_GPIO > 0
    else if(strcasecmp_P(topic, PSTR("gpio")) == 0) {
        if(update)
            gpioSetConfig(settings);
        else
            gpioGetConfig(settings);
    }
#endif

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
        char subtopic[8];
        settings.remove(F("pass")); // hide password in output

        /* size_t size = */ serializeJson(doc, buffer, sizeof(buffer));
        memcpy_P(subtopic, PSTR("config"), 7);
        dispatch_state_subtopic(subtopic, buffer);
    }
}
#endif // HASP_USE_CONFIG

/********************************************** Output States ******************************************/
/*
static inline void dispatch_state_msg(const __FlashStringHelper* subtopic, const char* payload)
{

}*/

// void dispatch_group_onoff(uint8_t groupid, uint16_t eventid, lv_obj_t * obj)
// {
//     if((eventid == HASP_EVENT_LONG) || (eventid == HASP_EVENT_HOLD)) return; // don't send repeat events

//     if(groupid >= 0) {
//         bool state = Parser::get_event_state(eventid);
//         gpio_set_group_onoff(groupid, state);
//         object_set_normalized_group_value(groupid, eventid, obj);
//     }

//     char payload[8];
//     Parser::get_event_name(eventid, payload, sizeof(payload));
//     // dispatch_output_group_state(groupid, payload);
// }

// void dispatch_group_value(uint8_t groupid, int16_t state, lv_obj_t * obj)
// {
//     if(groupid >= 0) {
//         gpio_set_group_value(groupid, state);
//         object_set_normalized_group_value(groupid, state, obj);
//     }

//     char payload[8];
//     // dispatch_output_group_state(groupid, payload);
// }

void dispatch_normalized_group_value(uint8_t groupid, lv_obj_t* obj, int16_t val, int16_t min, int16_t max)
{
    if(groupid == 0) return;

    LOG_VERBOSE(TAG_MSGR, F("GROUP %d value %d (%d-%d)"), groupid, val, min, max);
#if HASP_USE_GPIO > 0
    gpio_set_normalized_group_value(groupid, val, min, max); // Update GPIO states
#endif
    object_set_normalized_group_value(groupid, obj, val, min, max); // Update onsreen objects
}

/********************************************** Native Commands ****************************************/

void dispatch_screenshot(const char*, const char* filename)
{
#if HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0

    if(strlen(filename) == 0) { // no filename given
        char tempfile[32];
        memcpy_P(tempfile, PSTR("/screenshot.bmp"), sizeof(tempfile));
        guiTakeScreenshot(tempfile);
    } else if(strlen(filename) > 31 || filename[0] != '/') { // Invalid filename
        LOG_WARNING(TAG_MSGR, "Invalid filename %s", filename);
    } else { // Valid filename
        guiTakeScreenshot(filename);
    }

#else
    LOG_WARNING(TAG_MSGR, D_FILE_SAVE_FAILED, filename);
#endif
}

void dispatch_parse_json(const char*, const char* payload)
{ // Parse an incoming JSON array into individual commands
    /*  if(strPayload.endsWith(",]")) {
          // Trailing null array elements are an artifact of older Home Assistant automations
          // and need to be removed before parsing by ArduinoJSON 6+
          strPayload.remove(strPayload.length() - 2, 2);
          strPayload.concat("]");
      }*/
    size_t maxsize = (128u * ((strlen(payload) / 128) + 1)) + 512;
    DynamicJsonDocument json(maxsize);

    // Note: Deserialization needs to be (const char *) so the objects WILL be copied
    // this uses more memory but otherwise the mqtt receive buffer can get overwritten by the send buffer !!
    DeserializationError jsonError = deserializeJson(json, payload);
    json.shrinkToFit();

    if(jsonError) { // Couldn't parse incoming JSON command
        dispatch_json_error(TAG_MSGR, jsonError);

    } else if(json.is<JsonArray>()) { // handle json as an array of commands
        JsonArray arr = json.as<JsonArray>();
        // guiStop();
        for(JsonVariant command : arr) {
            dispatch_text_line(command.as<const char*>());
        }
        // guiStart();
    } else if(json.is<JsonObject>()) { // handle json as a jsonl
        uint8_t savedPage = haspPages.get();
        hasp_new_object(json.as<JsonObject>(), savedPage);

        // #ifdef ARDUINO
        //     } else if(json.is<String>()) { // handle json as a single command
        //         dispatch_text_line(json.as<String>().c_str());
        // #else
    } else if(json.is<std::string>()) { // handle json as a single command
        dispatch_text_line(json.as<std::string>().c_str());
        // #endif

    } else if(json.is<const char*>()) { // handle json as a single command
        dispatch_text_line(json.as<const char*>());

    } else if(json.is<char*>()) { // handle json as a single command
        dispatch_text_line(json.as<char*>());

    } else {
        LOG_WARNING(TAG_MSGR, F(D_DISPATCH_COMMAND_NOT_FOUND), payload);
    }
}

#ifdef ARDUINO
void dispatch_parse_jsonl(Stream& stream)
#else
void dispatch_parse_jsonl(std::istream& stream)
#endif
{
    uint8_t savedPage = haspPages.get();
    size_t line       = 1;
    DynamicJsonDocument jsonl(MQTT_MAX_PACKET_SIZE / 2 + 128); // max ~256 characters per line
    DeserializationError jsonError = deserializeJson(jsonl, stream);

#ifdef ARDUINO
    stream.setTimeout(25);
#endif

    // guiStop();
    while(jsonError == DeserializationError::Ok) {
        hasp_new_object(jsonl.as<JsonObject>(), savedPage);
        jsonError = deserializeJson(jsonl, stream);
        line++;
    }
    // guiStart();

    /* For debugging pourposes */
    if(jsonError == DeserializationError::EmptyInput) {
        LOG_INFO(TAG_MSGR, F(D_JSONL_SUCCEEDED));

    } else {
        LOG_ERROR(TAG_MSGR, F(D_JSONL_FAILED ": %s"), line, jsonError.c_str());
    }
}

void dispatch_parse_jsonl(const char*, const char* payload)
{
#if HASP_USE_CONFIG > 0
    CharStream stream((char*)payload);
    // stream.setTimeout(10);
    dispatch_parse_jsonl(stream);
#else
    std::istringstream stream((char*)payload);
    dispatch_parse_jsonl(stream);
#endif
}

void dispatch_output_current_page()
{
    char topic[8];
    char payload[8];

    memcpy_P(topic, PSTR("page"), 5);
    snprintf_P(payload, sizeof(payload), PSTR("%d"), haspPages.get());
    dispatch_state_subtopic(topic, payload);
}

// Dispatch Page Get or Set
void dispatch_page_next(lv_scr_load_anim_t animation)
{
    haspPages.next(animation);
    dispatch_output_current_page();
}

void dispatch_page_prev(lv_scr_load_anim_t animation)
{
    haspPages.prev(animation);
    dispatch_output_current_page();
}

void dispatch_page_back(lv_scr_load_anim_t animation)
{
    haspPages.back(animation);
    dispatch_output_current_page();
}

void dispatch_set_page(uint8_t pageid)
{
    dispatch_set_page(pageid, LV_SCR_LOAD_ANIM_NONE);
}

void dispatch_set_page(uint8_t pageid, lv_scr_load_anim_t animation)
{
    haspPages.set(pageid, animation);
    dispatch_output_current_page();
}

void dispatch_page(const char*, const char* page)
{
    if(strlen(page) == 0) {
        dispatch_output_current_page(); // No payload, send current page
        return;
    }

    lv_scr_load_anim_t animation = LV_SCR_LOAD_ANIM_NONE;
    if(Parser::is_only_digits(page)) {
        uint8_t pageid = atoi(page);
        dispatch_set_page(pageid, animation);
    } else if(!strcasecmp_P(page, PSTR("prev"))) {
        dispatch_page_prev(animation);
    } else if(!strcasecmp_P(page, PSTR("next"))) {
        dispatch_page_next(animation);
    } else if(!strcasecmp_P(page, PSTR("back"))) {
        dispatch_page_back(animation);
    } else {
        LOG_WARNING(TAG_MSGR, PSTR(D_DISPATCH_INVALID_PAGE), page);
    }
}

// Clears a page id or the current page if empty
void dispatch_clear_page(const char*, const char* page)
{
    uint8_t pageid;
    if(strlen(page) > 0) {
        if(!strcasecmp_P(page, PSTR("all"))) {
            for(pageid = 0; pageid < HASP_NUM_PAGES; pageid++) haspPages.clear(pageid);
        } else {
            pageid = atoi(page);
        }
    } else {
        pageid = haspPages.get();
    }
    haspPages.clear(pageid);
}

void dispatch_dim(const char*, const char* level)
{
    // Set the current state
    if(strlen(level) != 0) haspDevice.set_backlight_level(atoi(level));

    char topic[8];
    char payload[8];

    memcpy_P(topic, PSTR("dim"), 4);
    snprintf_P(payload, sizeof(payload), PSTR("%d"), haspDevice.get_backlight_level());
    dispatch_state_subtopic(topic, payload);
}

void dispatch_moodlight(const char* topic, const char* payload)
{
    // Set the current state
    if(strlen(payload) != 0) {

        size_t maxsize = (128u * ((strlen(payload) / 128) + 1)) + 512;
        DynamicJsonDocument json(maxsize);

        // Note: Deserialization needs to be (const char *) so the objects WILL be copied
        // this uses more memory but otherwise the mqtt receive buffer can get overwritten by the send buffer !!
        DeserializationError jsonError = deserializeJson(json, payload);
        json.shrinkToFit();

        if(jsonError) { // Couldn't parse incoming JSON command
            dispatch_json_error(TAG_MSGR, jsonError);
        } else {

            if(!json[F("state")].isNull())
                moodlight.power = Parser::is_true(json[F("state")].as<std::string>().c_str());

            if(!json["r"].isNull()) moodlight.r = json["r"].as<uint8_t>();
            if(!json["g"].isNull()) moodlight.g = json["g"].as<uint8_t>();
            if(!json["b"].isNull()) moodlight.b = json["b"].as<uint8_t>();

            if(!json[F("color")].isNull()) {
                if(!json[F("color")]["r"].isNull()) {
                    moodlight.r = json[F("color")]["r"].as<uint8_t>();
                }
                if(!json[F("color")]["g"].isNull()) {
                    moodlight.g = json[F("color")]["g"].as<uint8_t>();
                }
                if(!json[F("color")]["b"].isNull()) {
                    moodlight.b = json[F("color")]["b"].as<uint8_t>();
                }
                // lv_color32_t color;
                // if(Parser::haspPayloadToColor(json[F("color")].as<const char*>(), color)) {
                //     moodlight.r = color.ch.red;
                //     moodlight.g = color.ch.green;
                //     moodlight.b = color.ch.blue;
                // }
            }

#if HASP_USE_GPIO > 0
            if(moodlight.power)
                gpio_set_moodlight(moodlight.r, moodlight.g, moodlight.b);
            else
                gpio_set_moodlight(0, 0, 0);
#endif
        }
    }

    // Return the current state
    char buffer[128];
    char out_topic[16];
    memcpy_P(out_topic, PSTR("moodlight"), 10);
    snprintf_P(
        // buffer, sizeof(buffer),
        // PSTR("{\"state\":\"%s\",\"color\":\"#%02x%02x%02x\",\"r\":%u,\"g\":%u,\"b\":%u}"),
        buffer, sizeof(buffer), PSTR("{\"state\":\"%s\",\"color\":{\"r\":%u,\"g\":%u,\"b\":%u}}"),
        moodlight.power ? "ON" : "OFF", moodlight.r, moodlight.g, moodlight.b);
    dispatch_state_subtopic(out_topic, buffer);
}

void dispatch_backlight(const char*, const char* payload)
{
    // Set the current state
    if(strlen(payload) != 0) {
        bool power = Parser::is_true(payload);

        if(haspDevice.get_backlight_power() != power) {
            haspDevice.set_backlight_power(power);
            if(power)
                hasp_disable_wakeup_touch();
            else
                hasp_enable_wakeup_touch();
        }
    }

    // Return the current state
    char topic[8];
    char buffer[4];
    memcpy_P(topic, PSTR("light"), 6);
    memcpy_P(buffer, haspDevice.get_backlight_power() ? PSTR("on") : PSTR("off"), sizeof(buffer));
    dispatch_state_subtopic(topic, buffer);
}

void dispatch_web_update(const char*, const char* espOtaUrl)
{
#if HASP_USE_OTA > 0
    LOG_TRACE(TAG_MSGR, F(D_OTA_CHECK_UPDATE), espOtaUrl);
    otaHttpUpdate(espOtaUrl);
#endif
}

// restart the device
void dispatch_reboot(bool saveConfig)
{
#if HASP_USE_CONFIG > 0
    if(saveConfig) configWrite();
#endif
#if HASP_USE_MQTT > 0 && defined(ARDUINO)
    mqttStop(); // Stop the MQTT Client first
#endif
#if HASP_USE_CONFIG > 0
    debugStop();
#endif
#if HASP_USE_WIFI > 0
    wifiStop();
#endif
    LOG_VERBOSE(TAG_MSGR, F("-------------------------------------"));
    LOG_TRACE(TAG_MSGR, F(D_DISPATCH_REBOOT));

#if defined(WINDOWS) || defined(POSIX)
    fflush(stdout);
#else
    Serial.flush();
#endif

    // halRestartMcu();
    haspDevice.reboot();
}

void dispatch_current_state()
{
    dispatch_output_statusupdate(NULL, NULL);
    dispatch_output_idle_state(hasp_sleep_state);
    dispatch_output_current_page();
}

/******************************************* Command Wrapper Functions *********************************/

// Periodically publish a JSON string indicating system status
void dispatch_send_discovery(const char*, const char*)
{
#if HASP_USE_MQTT > 0

    char data[512];
    {
        char buffer[128];

        haspGetVersion(buffer, sizeof(buffer));
        snprintf_P(data, sizeof(data),
                   PSTR("{\"node\":\"%s\",\"manufacturer\":\"" D_MANUFACTURER
                        "\",\"model\":\"%s\",\"hwid\":\"%s\",\"version\":\"%s\",\"numPages\":%u}"),
                   haspDevice.get_hostname(), haspDevice.get_model(), haspDevice.get_hardware_id(), buffer,
                   haspPages.count());
    }

    switch(mqtt_send_discovery(data)) {
        case MQTT_ERR_OK:
            LOG_TRACE(TAG_MQTT_PUB, F("discovery => %s"), data);
            break;
        case MQTT_ERR_PUB_FAIL:
            LOG_ERROR(TAG_MQTT_PUB, F(D_MQTT_FAILED " discovery => %s"), data);
            break;
        case MQTT_ERR_NO_CONN:
            LOG_ERROR(TAG_MQTT, F(D_MQTT_NOT_CONNECTED));
            break;
        default:
            LOG_ERROR(TAG_MQTT, F(D_ERROR_UNKNOWN));
    }
    dispatchLastMillis = millis();

#endif
}

// Periodically publish a JSON string indicating system status
void dispatch_output_statusupdate(const char*, const char*)
{
#if HASP_USE_MQTT > 0

    char data[400];
    char topic[16];
    {
        char buffer[128];

        haspGetVersion(buffer, sizeof(buffer));
        dispatch_get_idle_state(hasp_sleep_state, topic);
        snprintf_P(data, sizeof(data), PSTR("{\"node\":\"%s\",\"idle\":\"%s\",\"version\":\"%s\",\"uptime\":%lu,"),
                   haspDevice.get_hostname(), topic, buffer, long(millis() / 1000)); // \"status\":\"available\",

#if HASP_USE_WIFI > 0 || HASP_USE_ETHERNET > 0
        network_get_statusupdate(buffer, sizeof(buffer));
        strcat(data, buffer);

        snprintf_P(buffer, sizeof(buffer), PSTR("\"mac\":\"%s\","), halGetMacAddress(0, ":").c_str());
        strcat(data, buffer);
#endif

        snprintf_P(buffer, sizeof(buffer), PSTR("\"heapFree\":%zu,\"heapFrag\":%u,\"core\":\"%s\","),
                   haspDevice.get_free_heap(), haspDevice.get_heap_fragmentation(), haspDevice.get_core_version());
        strcat(data, buffer);

        snprintf_P(buffer, sizeof(buffer), PSTR("\"canUpdate\":\"false\",\"page\":%u,\"numPages\":%zu,"),
                   haspPages.get(), haspPages.count());
        strcat(data, buffer);

        // #if defined(ARDUINO_ARCH_ESP8266)
        //         snprintf_P(buffer, sizeof(buffer), PSTR("\"espVcc\":%.2f,"), (float)ESP.getVcc() / 1000);
        //         strcat(data, buffer);
        // #endif

        snprintf_P(buffer, sizeof(buffer), PSTR("\"tftDriver\":\"%s\",\"tftWidth\":%u,\"tftHeight\":%u}"),
                   haspTft.get_tft_model(), (TFT_WIDTH), (TFT_HEIGHT));
        strcat(data, buffer);
    }

    memcpy_P(topic, PSTR("statusupdate"), 13);
    dispatch_state_subtopic(topic, data);
    dispatchLastMillis = millis();

    /* if(updateEspAvailable) {
            mqttStatusPayload += F("\"updateEspAvailable\":true,");
        } else {
            mqttStatusPayload += F("\"updateEspAvailable\":false,");
        }
    */

#endif
}

void dispatch_calibrate(const char* topic = NULL, const char* payload = NULL)
{
    guiCalibrate();
}

void dispatch_wakeup(const char*, const char*)
{
    lv_disp_trig_activity(NULL);
    hasp_disable_wakeup_touch();
}

void dispatch_sleep(const char*, const char*)
{
    hasp_enable_wakeup_touch();
}

void dispatch_reboot(const char*, const char*)
{
    dispatch_reboot(true);
}

void dispatch_factory_reset(const char*, const char*)
{
    dispatch_factory_reset();
    delay(500);
    dispatch_reboot(false); // don't save running config
}

/******************************************* Commands builder *******************************************/

static void dispatch_add_command(const char* p_cmdstr, void (*func)(const char*, const char*))
{
    if(nCommands >= sizeof(commands) / sizeof(haspCommand_t)) {
        LOG_FATAL(TAG_MSGR, F("CMD_OVERFLOW %d"), nCommands);
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
    // The command.func() call will receive the full topic and payload parameters!

    /* WARNING: remember to expand the commands array when adding new commands */
    dispatch_add_command(PSTR("json"), dispatch_parse_json);
    dispatch_add_command(PSTR("page"), dispatch_page);
    dispatch_add_command(PSTR("wakeup"), dispatch_wakeup);
    dispatch_add_command(PSTR("sleep"), dispatch_sleep);
    dispatch_add_command(PSTR("statusupdate"), dispatch_output_statusupdate);
    dispatch_add_command(PSTR("clearpage"), dispatch_clear_page);
    dispatch_add_command(PSTR("jsonl"), dispatch_parse_jsonl);
    dispatch_add_command(PSTR("dim"), dispatch_dim);
    dispatch_add_command(PSTR("brightness"), dispatch_dim);
    dispatch_add_command(PSTR("light"), dispatch_backlight);
    dispatch_add_command(PSTR("moodlight"), dispatch_moodlight);
    dispatch_add_command(PSTR("calibrate"), dispatch_calibrate);
    dispatch_add_command(PSTR("update"), dispatch_web_update);
    dispatch_add_command(PSTR("reboot"), dispatch_reboot);
    dispatch_add_command(PSTR("restart"), dispatch_reboot);
    dispatch_add_command(PSTR("screenshot"), dispatch_screenshot);
    dispatch_add_command(PSTR("factoryreset"), dispatch_factory_reset);
#if HASP_USE_CONFIG > 0
    dispatch_add_command(PSTR("setupap"), oobeFakeSetup);
#endif
    /* WARNING: remember to expand the commands array when adding new commands */
}

void dispatchLoop()
{
    lv_task_handler(); // process animations
}

#if 1 || ARDUINO
void dispatchEverySecond()
{
    if(dispatch_setings.teleperiod > 0 && (millis() - dispatchLastMillis) >= dispatch_setings.teleperiod * 1000) {
        dispatchLastMillis += dispatch_setings.teleperiod * 1000;
        dispatch_output_statusupdate(NULL, NULL);
        dispatch_send_discovery(NULL, NULL);
    }
}
#else
#include <chrono>
std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
void everySecond()
{
    if(dispatch_setings.teleperiod > 0) {
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::chrono::seconds elapsed = std::chrono::duration_cast<std::chrono::seconds>(end - begin);

        if(elapsed.count() >= dispatch_setings.teleperiod) {
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            dispatch_output_statusupdate(NULL, NULL);
        }
    }
}
#endif

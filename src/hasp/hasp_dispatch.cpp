/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include <stdint.h>

//#include "ArduinoLog.h"
#include "hasplib.h"

#include "dev/device.h"

//#include "hasp_gui.h"

#if HASP_USE_DEBUG > 0
#include "../hasp_debug.h"
#include "hasp_gui.h" // for screenshot

#if WINDOWS
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

dispatch_conf_t dispatch_setings = {.teleperiod = 10};

uint32_t dispatchLastMillis;
uint8_t nCommands = 0;
haspCommand_t commands[17];

struct moodlight_t
{
    uint8_t power;
    uint8_t r, g, b;
};
moodlight_t moodlight;

static void dispatch_config(const char* topic, const char* payload);
// void dispatch_group_value(uint8_t groupid, int16_t state, lv_obj_t * obj);
static inline void dispatch_state_msg(const __FlashStringHelper* subtopic, const char* payload);

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
    LOG_WARNING(TAG_MSGR, "Failed to save %s, no storage", filename);
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

    if(strlen(topic) == 7 && topic == strstr_P(topic, PSTR("output"))) {
        int16_t state = atoi(payload);
        dispatch_normalized_group_value(atoi(topic + 6), state, NULL); // + 6 => trim 'output' from the topic

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
              !strcmp_P(topic, PSTR("mqttport")) || !strcmp_P(topic, PSTR("mqttuser")) ||
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

// send idle state to the client
void dispatch_output_idle_state(uint8_t state)
{
    char payload[6];
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
    dispatch_state_msg(F("idle"), payload);
}

void dispatch_output_group_state(uint8_t groupid, uint16_t state)
{
    char payload[64];
    char number[16]; // Return the current state
    itoa(state, number, DEC);
    snprintf_P(payload, sizeof(payload), PSTR("{\"group\":%d,\"state\":\"%s\"}"), groupid, number);

    dispatch_state_msg(F("output"), payload);
}

void dispatch_send_obj_attribute_str(uint8_t pageid, uint8_t btnid, const char* attribute, const char* data)
{
    if(!attribute || !data) return;

    char payload[32 + strlen(data) + strlen(attribute)];
    snprintf_P(payload, sizeof(payload), PSTR("{\"%s\":\"%s\"}"), attribute, data);

    mqtt_send_object_state(pageid, btnid, payload);
}

void dispatch_send_obj_attribute_int(uint8_t pageid, uint8_t btnid, const char* attribute, int32_t val)
{
    if(!attribute) return;

    char payload[64 + strlen(attribute)];
    snprintf_P(payload, sizeof(payload), PSTR("{\"%s\":%d}"), attribute, val);

    mqtt_send_object_state(pageid, btnid, payload);
}

void dispatch_send_obj_attribute_color(uint8_t pageid, uint8_t btnid, const char* attribute, uint8_t r, uint8_t g,
                                       uint8_t b)
{
    if(!attribute) return;

    char payload[64 + strlen(attribute)];
    snprintf_P(payload, sizeof(payload), PSTR("{\"%s\":\"#%02x%02x%02x\",\"r\":%d,\"g\":%d,\"b\":%d}"), attribute, r, g,
               b, r, g, b);

    mqtt_send_object_state(pageid, btnid, payload);
}

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
        settings.remove(F("pass")); // hide password in output
        size_t size = serializeJson(doc, buffer, sizeof(buffer));
        dispatch_state_msg(F("config"), buffer);
    }
}
#endif // HASP_USE_CONFIG

/********************************************** Input Events *******************************************/
// Map events to either ON or OFF (UP or DOWN)
bool dispatch_get_event_state(uint8_t eventid)
{
    switch(eventid) {
        case HASP_EVENT_ON:
        case HASP_EVENT_DOWN:
        case HASP_EVENT_LONG:
        case HASP_EVENT_HOLD:
            return true;
        // case HASP_EVENT_OFF:
        // case HASP_EVENT_UP:
        // case HASP_EVENT_SHORT:
        // case HASP_EVENT_DOUBLE:
        // case HASP_EVENT_LOST:
        default:
            return false;
    }
}

// Map events to their description string
void dispatch_get_event_name(uint8_t eventid, char* buffer, size_t size)
{
    switch(eventid) {
        case HASP_EVENT_ON:
            memcpy_P(buffer, PSTR("on"), size);
            break;
        case HASP_EVENT_OFF:
            memcpy_P(buffer, PSTR("off"), size);
            break;
        case HASP_EVENT_UP:
            memcpy_P(buffer, PSTR("up"), size);
            break;
        case HASP_EVENT_DOWN:
            memcpy_P(buffer, PSTR("down"), size);
            break;
        case HASP_EVENT_SHORT:
            memcpy_P(buffer, PSTR("short"), size);
            break;
        case HASP_EVENT_LONG:
            memcpy_P(buffer, PSTR("long"), size);
            break;
        case HASP_EVENT_HOLD:
            memcpy_P(buffer, PSTR("hold"), size);
            break;
        case HASP_EVENT_LOST:
            memcpy_P(buffer, PSTR("lost"), size);
            break;
        default:
            memcpy_P(buffer, PSTR("unknown"), size);
    }
}

#if HASP_USE_GPIO > 0
void dispatch_gpio_input_event(uint8_t pin, uint8_t group, uint8_t eventid)
{
    char payload[64];
    char event[8];
    dispatch_get_event_name(eventid, event, sizeof(event));
    snprintf_P(payload, sizeof(payload), PSTR("{\"pin\":%d,\"group\":%d,\"event\":\"%s\"}"), pin, group, event);

#if HASP_USE_MQTT > 0
    mqtt_send_state(F("input"), payload);
#endif

    // update outputstates
    // dispatch_group_onoff(group, dispatch_get_event_state(eventid), NULL);
}
#endif

void dispatch_object_event(lv_obj_t* obj, uint8_t eventid)
{
    char topic[8];
    char payload[8];
    uint8_t pageid, objid;

    snprintf_P(topic, sizeof(topic), PSTR("event"));
    dispatch_get_event_name(eventid, payload, sizeof(payload));

    if(hasp_find_id_from_obj(obj, &pageid, &objid)) {
        dispatch_send_obj_attribute_str(pageid, objid, topic, payload);
    }

    //  dispatch_group_onoff(obj->user_data.groupid, dispatch_get_event_state(eventid), obj);
}

void dispatch_object_value_changed(lv_obj_t* obj, int16_t state)
{
    char topic[4];

    hasp_update_sleep_state(); // wakeup?
    snprintf_P(topic, sizeof(topic), PSTR("val"));
    hasp_send_obj_attribute_int(obj, topic, state);
}

/********************************************** Output States ******************************************/
static inline void dispatch_state_msg(const __FlashStringHelper* subtopic, const char* payload)
{
#if !defined(HASP_USE_MQTT) && !defined(HASP_USE_TASMOTA_CLIENT)
    LOG_TRACE(TAG_MSGR, F("%s => %s"), String(subtopic).c_str(), payload);
#else
#if HASP_USE_MQTT > 0
    mqtt_send_state(subtopic, payload);
#endif
#if HASP_USE_TASMOTA_CLIENT > 0
    slave_send_state(subtopic, payload);
#endif
#endif
}

// void dispatch_group_onoff(uint8_t groupid, uint16_t eventid, lv_obj_t * obj)
// {
//     if((eventid == HASP_EVENT_LONG) || (eventid == HASP_EVENT_HOLD)) return; // don't send repeat events

//     if(groupid >= 0) {
//         bool state = dispatch_get_event_state(eventid);
//         gpio_set_group_onoff(groupid, state);
//         object_set_group_state(groupid, eventid, obj);
//     }

//     char payload[8];
//     dispatch_get_event_name(eventid, payload, sizeof(payload));
//     // dispatch_output_group_state(groupid, payload);
// }

// void dispatch_group_value(uint8_t groupid, int16_t state, lv_obj_t * obj)
// {
//     if(groupid >= 0) {
//         gpio_set_group_value(groupid, state);
//         object_set_group_state(groupid, state, obj);
//     }

//     char payload[8];
//     // dispatch_output_group_state(groupid, payload);
// }

void dispatch_normalized_group_value(uint8_t groupid, uint16_t value, lv_obj_t* obj)
{
    if(groupid > 0) {
        LOG_VERBOSE(TAG_MSGR, F("GROUP %d value %d"), groupid, value);
#if HASP_USE_GPIO > 0
        gpio_set_normalized_group_value(groupid, value);
#endif
        //  object_set_group_state(groupid, value, obj);
    }
}

/********************************************** Native Commands ****************************************/

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
        uint8_t savedPage = haspGetPage();
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
void dispatch_parse_jsonl(std::istringstream& stream)
#endif
{
    uint8_t savedPage = haspGetPage();
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
    // Log result
    char payload[4];
    itoa(haspGetPage(), payload, DEC);
    dispatch_state_msg(F("page"), payload);
}

// Get or Set a page
void dispatch_page(const char*, const char* page)
{
    if(strlen(page) > 0) {
        if(Utilities::is_only_digits(page)) {
            uint8_t pageid = atoi(page);
            haspSetPage(pageid);
        } else {

            if(!strcasecmp_P(page, PSTR("prev"))) {
                dispatch_page_prev();
            } else if(!strcasecmp_P(page, PSTR("next"))) {
                dispatch_page_next();
            } else {
                LOG_WARNING(TAG_MSGR, PSTR(D_DISPATCH_INVALID_PAGE), page);
            }
            return;
        }
    }

    dispatch_output_current_page();
}

void dispatch_page_next()
{
    uint8_t page = haspGetPage();
    if(page >= HASP_NUM_PAGES) {
        page = 1;
    } else {
        page++;
    }
    haspSetPage(page);
    dispatch_output_current_page();
}

void dispatch_page_prev()
{
    uint8_t page = haspGetPage();
    if(page == 1) {
        page = HASP_NUM_PAGES;
    } else {
        page--;
    }
    haspSetPage(page);
    dispatch_output_current_page();
}

// Clears a page id or the current page if empty
void dispatch_clear_page(const char*, const char* page)
{
    uint8_t pageid = haspGetPage();
    if(strlen(page) > 0) pageid = atoi(page);
    haspClearPage(pageid);
}

void dispatch_dim(const char*, const char* level)
{
    // Set the current state
    if(strlen(level) != 0) haspDevice.set_backlight_level(atoi(level));

    char payload[5];
    itoa(haspDevice.get_backlight_level(), payload, DEC);
    dispatch_state_msg(F("dim"), payload);
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
                moodlight.power = Utilities::is_true(json[F("state")].as<std::string>().c_str());

            if(!json[F("r")].isNull()) moodlight.r = json[F("r")].as<uint8_t>();
            if(!json[F("g")].isNull()) moodlight.g = json[F("g")].as<uint8_t>();
            if(!json[F("b")].isNull()) moodlight.b = json[F("b")].as<uint8_t>();

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
    snprintf_P(
        // buffer, sizeof(buffer), PSTR("{\"state\":\"%s\",\"color\":\"#%02x%02x%02x\",\"r\":%u,\"g\":%u,\"b\":%u}"),
        buffer, sizeof(buffer), PSTR("{\"state\":\"%s\",\"color\":{\"r\":%u,\"g\":%u,\"b\":%u}}"),
        moodlight.power ? "ON" : "OFF", moodlight.r, moodlight.g, moodlight.b);
    dispatch_state_msg(F("moodlight"), buffer);
}

void dispatch_backlight(const char*, const char* payload)
{
    // Set the current state
    if(strlen(payload) != 0) haspDevice.set_backlight_power(Utilities::is_true(payload));

    // Return the current state
    char buffer[4];
    memcpy_P(buffer, haspDevice.get_backlight_power() ? PSTR("ON") : PSTR("OFF"), sizeof(buffer));
    dispatch_state_msg(F("light"), buffer);
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

#if WINDOWS
    fflush(stdout);
#else
    Serial.flush();
#endif

    // halRestartMcu();
    haspDevice.reboot();
}

void dispatch_current_state()
{
    dispatch_output_current_page();
    dispatch_output_statusupdate(NULL, NULL);
    dispatch_output_idle_state(hasp_sleep_state);
}

/******************************************* Command Wrapper Functions *********************************/

// Periodically publish a JSON string indicating system status
void dispatch_output_statusupdate(const char*, const char*)
{
#if HASP_USE_MQTT > 0

    char data[3 * 128];
    {
        char buffer[128];

        haspGetVersion(buffer, sizeof(buffer));
        snprintf_P(data, sizeof(data),
                   PSTR("{\"node\":\"%s\",\"status\":\"available\",\"version\":\"%s\",\"uptime\":%lu,"),
                   haspDevice.get_hostname(), buffer, long(millis() / 1000));

#if HASP_USE_WIFI > 0
        network_get_statusupdate(buffer, sizeof(buffer));
        strcat(data, buffer);
#endif

        snprintf_P(buffer, sizeof(buffer), PSTR("\"heapFree\":%u,\"heapFrag\":%u,\"espCore\":\"%s\","),
                   haspDevice.get_free_heap(), haspDevice.get_heap_fragmentation(), haspDevice.get_core_version());
        strcat(data, buffer);

        snprintf_P(buffer, sizeof(buffer), PSTR("\"espCanUpdate\":\"false\",\"page\":%u,\"numPages\":%u,"),
                   haspGetPage(), (HASP_NUM_PAGES));
        strcat(data, buffer);

#if defined(ARDUINO_ARCH_ESP8266)
        snprintf_P(buffer, sizeof(buffer), PSTR("\"espVcc\":%.2f,"), (float)ESP.getVcc() / 1000);
        strcat(data, buffer);
#endif

        snprintf_P(buffer, sizeof(buffer), PSTR("\"tftDriver\":\"%s\",\"tftWidth\":%u,\"tftHeight\":%u}"),
                   Utilities::tft_driver_name().c_str(), (TFT_WIDTH), (TFT_HEIGHT));
        strcat(data, buffer);
    }
    mqtt_send_state(F("statusupdate"), data);
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
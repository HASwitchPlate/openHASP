/* MIT License - Copyright (c) 2019-2022 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include <time.h>
#include <sys/time.h>

//#include "ArduinoLog.h"
#include "hasplib.h"

#include "dev/device.h"
#include "drv/tft/tft_driver.h"

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
#include "StreamUtils.h" // for exec ReadBufferingStream
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

#if defined(HASP_USE_CUSTOM)
#include "custom/my_custom.h"
#endif

dispatch_conf_t dispatch_setings = {.teleperiod = 300};

uint16_t dispatchSecondsToNextTeleperiod = 0;
uint8_t nCommands                        = 0;
haspCommand_t commands[26];

moodlight_t moodlight = {.brightness = 255};

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
            LOG_ERROR(TAG_MQTT, F(D_MQTT_NOT_CONNECTED " %s => %s"), subtopic, payload);
            break;
        default:
            LOG_ERROR(TAG_MQTT, F(D_ERROR_UNKNOWN " %s => %s"), subtopic, payload);
    }
#endif

#if HASP_USE_TASMOTA_CLIENT > 0
    slave_send_state(subtopic, payload);
#endif

#endif
}

void dispatch_state_eventid(const char* topic, hasp_event_t eventid)
{
    char payload[32];
    char eventname[8];

    Parser::get_event_name(eventid, eventname, sizeof(eventname));
    if(eventid == HASP_EVENT_ON || eventid == HASP_EVENT_OFF) {
        snprintf_P(payload, sizeof(payload), PSTR("{\"state\":\"%s\"}"), eventname);
    } else {
        snprintf_P(payload, sizeof(payload), PSTR("{\"event\":\"%s\"}"), eventname);
    }
    dispatch_state_subtopic(topic, payload);
}

void dispatch_state_brightness(const char* topic, hasp_event_t eventid, int32_t val)
{
    char payload[64];
    char eventname[8];

    Parser::get_event_name(eventid, eventname, sizeof(eventname));
    snprintf_P(payload, sizeof(payload), PSTR("{\"state\":\"%s\",\"brightness\":%d}"), eventname, val);
    dispatch_state_subtopic(topic, payload);
}

void dispatch_state_antiburn(hasp_event_t eventid)
{
    char topic[9];
    char payload[64];
    char eventname[8];

    Parser::get_event_name(eventid, eventname, sizeof(eventname));
    snprintf_P(topic, sizeof(topic), PSTR("antiburn"));
    snprintf_P(payload, sizeof(payload), PSTR("{\"state\":\"%s\"}"), eventname);
    dispatch_state_subtopic(topic, payload);
}

void dispatch_state_val(const char* topic, hasp_event_t eventid, int32_t val)
{
    char payload[64];
    char eventname[8];

    Parser::get_event_name(eventid, eventname, sizeof(eventname));
    snprintf_P(payload, sizeof(payload), PSTR("{\"state\":\"%s\",\"val\":%d}"), eventname, val);
    dispatch_state_subtopic(topic, payload);
}

void dispatch_json_error(uint8_t tag, DeserializationError& jsonError)
{
    LOG_ERROR(tag, F(D_JSON_FAILED " %s"), jsonError.c_str());
}

// p[x].b[y].attr=value
static inline bool dispatch_parse_button_attribute(const char* topic_p, const char* payload, bool update)
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

    hasp_process_attribute(pageid, objid, topic_p, payload, update);
    return true;
}

static void dispatch_input(const char* topic, const char* payload)
{
#if HASP_USE_GPIO > 0

    if(!Parser::is_only_digits(topic)) {
        LOG_WARNING(TAG_MSGR, F("Invalid pin %s"), topic);
        return;
    }

    // just output the pin state
    uint8_t pin = atoi(topic);
    if(gpio_input_pin_state(pin)) return;

    LOG_WARNING(TAG_GPIO, F(D_BULLET "Pin %d is not configured"), pin);

#endif
}

static void dispatch_output(const char* topic, const char* payload)
{
#if HASP_USE_GPIO > 0

    if(!Parser::is_only_digits(topic)) {
        LOG_WARNING(TAG_MSGR, F("Invalid pin %s"), topic);
        return;
    }

    uint8_t pin = atoi(topic);

    if(strlen(payload) > 0) {

        size_t maxsize = (128u * ((strlen(payload) / 128) + 1)) + 128;
        DynamicJsonDocument json(maxsize);

        // Note: Deserialization needs to be (const char *) so the objects WILL be copied
        // this uses more memory but otherwise the mqtt receive buffer can get overwritten by the send buffer !!
        DeserializationError jsonError = deserializeJson(json, payload);
        json.shrinkToFit();

        if(jsonError) { // Couldn't parse incoming JSON command
            dispatch_json_error(TAG_MSGR, jsonError);
        } else {

            // Save the current state
            int32_t state_value;
            bool power_state;
            bool updated = false;

            if(!gpio_get_pin_state(pin, power_state, state_value)) {
                LOG_WARNING(TAG_GPIO, F(D_BULLET "Pin %d can not be set"), pin);
                return;
            }

            JsonVariant state      = json[F("state")];
            JsonVariant value      = json[F("val")];
            JsonVariant brightness = json[F("brightness")];

            // Check if the state needs to change
            if(!state.isNull() && power_state != Parser::is_true(state)) {
                power_state = Parser::is_true(state);
                updated     = true;
            }

            if(!value.isNull() && state_value != value.as<int32_t>()) {
                state_value = value.as<int32_t>();
                updated     = true;
            } else if(!brightness.isNull() && state_value != brightness.as<int32_t>()) {
                state_value = brightness.as<int32_t>();
                updated     = true;
            }

            // Set new state
            if(updated && gpio_set_pin_state(pin, power_state, state_value)) {
                return; // value was set and state output already in gpio_set_pin_state
            } else {
                // output the new state to the log
            }
        }
    }

    // just output this pin
    if(!gpio_output_pin_state(pin)) {
        LOG_WARNING(TAG_GPIO, F(D_BULLET "Pin %d is not configured"), pin);
    }

#endif
}

// objectattribute=value
void dispatch_command(const char* topic, const char* payload, bool update, uint8_t source)
{
    /* ================================= Standard payload commands ======================================= */

    if(dispatch_parse_button_attribute(topic, payload, update)) return; // matched pxby.attr, first for speed

    // check and execute commands from commands array
    for(int i = 0; i < nCommands; i++) {
        if(!strcasecmp_P(topic, commands[i].p_cmdstr)) {
            // LOG_DEBUG(TAG_MSGR, F("Command %d found in array !!!"), i);
            commands[i].func(topic, payload, source); /* execute command */
            return;
        }
    }

    /* =============================== Not standard payload commands ===================================== */

    if(topic == strstr_P(topic, PSTR("output"))) {
        dispatch_output(topic + 6, payload);

    } else if(topic == strstr_P(topic, PSTR("input"))) {
        dispatch_input(topic + 5, payload);

        // } else if(strcasecmp_P(topic, PSTR("screenshot")) == 0) {
        //     guiTakeScreenshot("/screenshot.bmp"); // Literal String

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
void dispatch_topic_payload(const char* topic, const char* payload, bool update, uint8_t source)
{
    if(!strcmp_P(topic, PSTR(MQTT_TOPIC_COMMAND))) {
        dispatch_text_line((char*)payload, source);
        return;
    }

    if(topic == strstr_P(topic, PSTR(MQTT_TOPIC_COMMAND "/"))) { // startsWith command/
        topic += 8u;
        dispatch_command(topic, (char*)payload, update, source);
        return;
    }

#if HASP_USE_CONFIG > 0
    if(topic == strstr_P(topic, PSTR("config/"))) { // startsWith config/
        topic += 7u;
        dispatch_config(topic, (char*)payload, source);
        return;
    }
#endif

#if defined(HASP_USE_CUSTOM)
    if(topic == strstr_P(topic, PSTR(MQTT_TOPIC_CUSTOM "/"))) { // startsWith custom
        topic += 7u;
        custom_topic_payload(topic, (char*)payload, source);
        return;
    }
#endif

    dispatch_command(topic, (char*)payload, update, source); // dispatch as is
}

// Parse one line of text and execute the command
void dispatch_text_line(const char* cmnd, uint8_t source)
{
    if(cmnd[0] == '/' && cmnd[1] == '/') return; // comment

    switch(cmnd[0]) {
        case '#':
            break; // comment

        case '{':
            dispatch_command("jsonl", cmnd, false, source);
            break;

        case '[':
            dispatch_command("json", cmnd, false, source);
            break; // comment

        case ' ':
            while(cmnd[0] == ' ') cmnd++; // skip leading spaces
            dispatch_text_line(cmnd, source);
            break;

        default: {
            size_t pos1 = std::string(cmnd).find("=");
            size_t pos2 = std::string(cmnd).find(" ");
            size_t pos  = 0;
            bool update = false;

            // Find what comes first, ' ' or '='
            if(pos1 != std::string::npos) {     // '=' found
                if(pos2 != std::string::npos) { // ' ' found
                    pos = (pos1 < pos2 ? pos1 : pos2);
                } else {
                    pos = pos1;
                }
                update = pos == pos1; // equal sign wins

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
                update |= strlen(cmnd + pos + 1) > 0; // equal sign OR space with payload
                LOG_TRACE(TAG_MSGR, update ? F("%s=%s") : F("%s%s"), topic, cmnd + pos + 1);
                dispatch_topic_payload(topic, cmnd + pos + 1, update, source);
            } else {
                char empty_payload[1] = {0};
                LOG_TRACE(TAG_MSGR, cmnd);
                dispatch_topic_payload(cmnd, empty_payload, false, source);
            }
        }
    }
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
void dispatch_config(const char* topic, const char* payload, uint8_t source)
{
    DynamicJsonDocument doc(128 * 3);
    char buffer[128 * 3];
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
#if HASP_USE_HTTP > 0 || HASP_USE_HTTP_ASYNC > 0
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

void dispatch_normalized_group_values(hasp_update_value_t& value)
{
    if(value.group == 0) return;

#if HASP_USE_GPIO > 0
    gpio_set_normalized_group_values(value); // Update GPIO states first
#endif
    object_set_normalized_group_values(value); // Update onsreen objects except originating obj

    LOG_VERBOSE(TAG_MSGR, F("GROUP %d value %d (%d-%d)"), value.group, value.val, value.min, value.max);
#if HASP_USE_GPIO > 0
    gpio_output_group_values(value.group); // Output new gpio values
#endif
}

/********************************************** Native Commands ****************************************/

void dispatch_screenshot(const char*, const char* filename, uint8_t source)
{
#if HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0

    if(strlen(filename) == 0) { // no filename given
        char tempfile[32];
        memcpy_P(tempfile, PSTR("/screenshot.bmp"), sizeof(tempfile));
        guiTakeScreenshot(tempfile);
    } else if(strlen(filename) > 31 || filename[0] != '/') { // Invalid filename
        LOG_WARNING(TAG_MSGR, F("D_FILE_SAVE_FAILED"), filename);
    } else { // Valid filename
        guiTakeScreenshot(filename);
    }

#else
    LOG_WARNING(TAG_MSGR, D_FILE_SAVE_FAILED, filename);
#endif
}

void dispatch_parse_json(const char*, const char* payload, uint8_t source)
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
            dispatch_text_line(command.as<const char*>(), source);
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
        dispatch_text_line(json.as<std::string>().c_str(), source);
        // #endif

    } else if(json.is<const char*>()) { // handle json as a single command
        dispatch_text_line(json.as<const char*>(), source);

        // } else if(json.is<char*>()) { // handle json as a single command
        //     dispatch_text_line(json.as<char*>());

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
    uint16_t line     = 1;
    DynamicJsonDocument jsonl(MQTT_MAX_PACKET_SIZE / 2 + 128);
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

void dispatch_parse_jsonl(const char*, const char* payload, uint8_t source)
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

void dispatch_exec(const char*, const char* payload, uint8_t source)
{
#if ARDUINO
#if HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0

    if(!HASP_FS.exists(payload)) {
        LOG_WARNING(TAG_MSGR, F(D_FILE_NOT_FOUND ": %s"), payload);
        return;
    }

    LOG_TRACE(TAG_MSGR, F(D_FILE_LOADING), payload);

    File cmdfile = HASP_FS.open(payload, "r");
    if(!cmdfile) {
        LOG_ERROR(TAG_MSGR, F(D_FILE_LOAD_FAILED), payload);
        return;
    }

    // char buffer[512]; // use stack
    String buffer((char*)0); // use heap
    buffer.reserve(256);

    ReadBufferingStream bufferedFile{cmdfile, 256};
    cmdfile.seek(0);

    while(bufferedFile.available()) {
        size_t index = 0;
        buffer       = "";
        // while(index < sizeof(buffer) - 1) {
        while(index < MQTT_MAX_PACKET_SIZE) {
            int c = bufferedFile.read();
            if(c < 0 || c == '\n' || c == '\r') { // CR or LF
                break;
            }
            // buffer[index] = (char)c;
            buffer += (char)c;
            index++;
        }
        // buffer[index] = 0;                                                      // terminate string
        // if(index > 0 && buffer[0] != '#') dispatch_text_line(buffer.c_str(), TAG_FILE); // # for comments
        if(index > 0 && buffer.charAt(0) != '#') dispatch_text_line(buffer.c_str(), TAG_FILE); // # for comments
    }

    cmdfile.close();
    LOG_INFO(TAG_MSGR, F(D_FILE_LOADED), payload);
#else
    LOG_INFO(TAG_MSGR, F(D_FILE_LOAD_FAILED), payload);
#endif
#else
    char path[strlen(payload) + 4];
    path[0] = '.';
    path[1] = '\0';
    strcat(path, payload);
    path[1] = '\\';

    LOG_TRACE(TAG_HASP, F("Loading %s from disk..."), path);
    std::ifstream f(path); // taking file as inputstream
    if(f) {
        std::string line;
        while(std::getline(f, line)) {
            LOG_VERBOSE(TAG_HASP, line.c_str());
            if(!line.empty() && line[0] != '#') dispatch_text_line(line.c_str(), TAG_FILE); // # for comments
        }
    } else {
        LOG_ERROR(TAG_MSGR, F(D_FILE_LOAD_FAILED), payload);
    }
    f.close();
    LOG_INFO(TAG_HASP, F("Loaded %s from disk"), path);
#endif
}

void dispatch_current_page()
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
    dispatch_current_page();
}

void dispatch_page_prev(lv_scr_load_anim_t animation)
{
    haspPages.prev(animation);
    dispatch_current_page();
}

void dispatch_page_back(lv_scr_load_anim_t animation)
{
    haspPages.back(animation);
    dispatch_current_page();
}

void dispatch_set_page(uint8_t pageid)
{
    dispatch_set_page(pageid, LV_SCR_LOAD_ANIM_NONE);
}

void dispatch_set_page(uint8_t pageid, lv_scr_load_anim_t animation)
{
    haspPages.set(pageid, animation);
    dispatch_current_page();
}

void dispatch_page(const char*, const char* page, uint8_t source)
{
    if(strlen(page) == 0) {
        dispatch_current_page(); // No payload, send current page
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
void dispatch_clear_page(const char*, const char* page, uint8_t source)
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

void dispatch_moodlight(const char* topic, const char* payload, uint8_t source)
{
    // Set the current state
    if(strlen(payload) != 0) {

        size_t maxsize = (128u * ((strlen(payload) / 128) + 1)) + 128;
        DynamicJsonDocument json(maxsize);

        // Note: Deserialization needs to be (const char *) so the objects WILL be copied
        // this uses more memory but otherwise the mqtt receive buffer can get overwritten by the send buffer !!
        DeserializationError jsonError = deserializeJson(json, payload);
        json.shrinkToFit();

        if(jsonError) { // Couldn't parse incoming JSON command
            dispatch_json_error(TAG_MSGR, jsonError);
        } else {
            JsonVariant state = json[F("state")];
            if(!state.isNull()) moodlight.power = Parser::is_true(state);

            if(!json["r"].isNull()) moodlight.rgbww[0] = json["r"].as<uint8_t>();
            if(!json["g"].isNull()) moodlight.rgbww[1] = json["g"].as<uint8_t>();
            if(!json["b"].isNull()) moodlight.rgbww[2] = json["b"].as<uint8_t>();
            if(!json["brightness"].isNull()) moodlight.brightness = json["brightness"].as<uint8_t>();

            if(!json[F("color")].isNull()) {
                lv_color32_t color;
                if(Parser::haspPayloadToColor(json[F("color")].as<const char*>(), color)) {
                    moodlight.rgbww[0] = color.ch.red;
                    moodlight.rgbww[1] = color.ch.green;
                    moodlight.rgbww[2] = color.ch.blue;
                }
            }

#if HASP_USE_GPIO > 0
            gpio_set_moodlight(moodlight);
#endif
        }
    }

    // Return the current state
    char buffer[128];
    char out_topic[16];
    memcpy_P(out_topic, PSTR("moodlight"), 10);
    snprintf_P(buffer, sizeof(buffer),
               PSTR("{\"state\":\"%s\",\"brightness\":%u,\"color\":\"#%02x%02x%02x\",\"r\":%u,\"g\":%u,\"b\":%u}"),
               moodlight.power ? "on" : "off", moodlight.brightness, moodlight.rgbww[0], moodlight.rgbww[1],
               moodlight.rgbww[2], moodlight.rgbww[0], moodlight.rgbww[1], moodlight.rgbww[2]);
    dispatch_state_subtopic(out_topic, buffer);
}

void dispatch_backlight_obsolete(const char* topic, const char* payload, uint8_t source)
{
    LOG_WARNING(TAG_MSGR, F(D_ATTRIBUTE_OBSOLETE D_ATTRIBUTE_INSTEAD), topic,
                "backlight"); // TODO: obsolete dim, light and brightness
    dispatch_backlight(topic, payload, source);
}

void dispatch_backlight(const char*, const char* payload, uint8_t source)
{
    bool power = haspDevice.get_backlight_power();

    // Set the current state
    if(strlen(payload) != 0) {
        size_t maxsize = (128u * ((strlen(payload) / 128) + 1)) + 128;
        DynamicJsonDocument json(maxsize);

        // Note: Deserialization needs to be (const char *) so the objects WILL be copied
        // this uses more memory but otherwise the mqtt receive buffer can get overwritten by the send buffer !!
        DeserializationError jsonError = deserializeJson(json, payload);
        json.shrinkToFit();

        if(jsonError) { // Couldn't parse incoming payload as json
            power = Parser::is_true(payload);

        } else {

            // plain numbers are parsed as valid json object
            if(json.is<uint8_t>()) {
                uint8_t level = json.as<uint8_t>();

                if(level <= 1)
                    power = level;
                else
                    haspDevice.set_backlight_level(level);

                // true and false are parsed as valid json object
            } else if(json.is<bool>()) {
                power = json.as<bool>();

            } else {
                JsonVariant state      = json[F("state")];
                JsonVariant brightness = json[F("brightness")];

                if(!state.isNull()) power = Parser::is_true(state);
                if(!brightness.isNull()) haspDevice.set_backlight_level(brightness.as<uint8_t>());
            }
        }
    }

    // toggle power and wakeup touch if changed
    if(haspDevice.get_backlight_power() != power) {
        haspDevice.set_backlight_power(power);
        hasp_set_wakeup_touch(!power);
    }

    // Return the current state
    char topic[10];
    memcpy_P(topic, PSTR("backlight"), 10);
    dispatch_state_brightness(topic, (hasp_event_t)haspDevice.get_backlight_power(), haspDevice.get_backlight_level());
}

void dispatch_web_update(const char*, const char* espOtaUrl, uint8_t source)
{
#if HASP_USE_OTA > 0
    LOG_TRACE(TAG_MSGR, F(D_OTA_CHECK_UPDATE), espOtaUrl);
    otaHttpUpdate(espOtaUrl);
#endif
}

void dispatch_antiburn(const char*, const char* payload, uint8_t source)
{
    if(strlen(payload) == 0) {
        dispatch_state_antiburn(hasp_get_antiburn());
        return;
    }

    size_t maxsize = (128u * ((strlen(payload) / 128) + 1)) + 128;
    DynamicJsonDocument json(maxsize);

    // Note: Deserialization needs to be (const char *) so the objects WILL be copied
    // this uses more memory but otherwise the mqtt receive buffer can get overwritten by the send buffer !!
    DeserializationError jsonError = deserializeJson(json, payload);
    json.shrinkToFit();
    bool state = false;

    if(jsonError) { // Couldn't parse incoming payload as json
        state = Parser::is_true(payload);
    } else {
        if(json.is<uint8_t>()) { // plain numbers are parsed as valid json object
            state = json.as<uint8_t>();

        } else if(json.is<bool>()) { // true and false are parsed as valid json object
            state = json.as<bool>();

        } else { // other text
            JsonVariant key = json[F("state")];
            if(!key.isNull()) state = Parser::is_true(key);
        }
    }

    hasp_set_antiburn(state ? 30 : 0, 1000); // ON = 30 cycles of 1000 milli seconds (i.e. 30 sec)
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
#if HASP_USE_TELNET > 0
    telnetStop();
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

/******************************************* Command Wrapper Functions *********************************/

// Periodically publish a JSON string indicating sensor status
void dispatch_send_sensordata(const char*, const char*, uint8_t source)
{
#if HASP_USE_MQTT > 0

    StaticJsonDocument<1024> doc;

    time_t rawtime;
    time(&rawtime);
    char buffer[80];
    struct tm* timeinfo = localtime(&rawtime);

    strftime(buffer, sizeof(buffer), "%FT%T", timeinfo);
    doc[F("time")] = buffer;

    long uptime         = haspDevice.get_uptime();
    doc[F("uptimeSec")] = (uint32_t)uptime;

    uint32_t seconds = uptime % 60;
    uint32_t minutes = uptime / 60;
    uint32_t hours   = minutes / 60;
    uint32_t days    = hours / 24;
    minutes          = minutes % 60;
    hours            = hours % 24;
    snprintf_P(buffer, sizeof(buffer), PSTR("%dT%02d:%02d:%02d"), days, hours, minutes, seconds);
    doc[F("uptime")] = buffer;

    haspDevice.get_sensors(doc);

#if defined(HASP_USE_CUSTOM)
    custom_get_sensors(doc);
#endif

    //     JsonObject input = doc.createNestedObject(F("input"));
    //     JsonArray relay  = doc.createNestedArray(F("power"));
    //     JsonArray led    = doc.createNestedArray(F("light"));
    //     JsonArray dimmer = doc.createNestedArray(F("dim"));

    // #if HASP_USE_GPIO > 0
    //     gpio_discovery(input, relay, led, dimmer);
    // #endif

    char data[1024];
    size_t len = serializeJson(doc, data);
    (void)len; // unused

    switch(mqtt_send_state(MQTT_TOPIC_SENSORS, data)) {
        case MQTT_ERR_OK:
            LOG_TRACE(TAG_MQTT_PUB, F(MQTT_TOPIC_SENSORS " => %s"), data);
            break;
        case MQTT_ERR_PUB_FAIL:
            LOG_ERROR(TAG_MQTT_PUB, F(D_MQTT_FAILED " " MQTT_TOPIC_SENSORS " => %s"), data);
            break;
        case MQTT_ERR_NO_CONN:
            LOG_ERROR(TAG_MQTT, F(D_MQTT_NOT_CONNECTED));
            break;
        default:
            LOG_ERROR(TAG_MQTT, F(D_ERROR_UNKNOWN));
    }

#endif
}

// Periodically publish a JSON string facilitating plate discovery
void dispatch_send_discovery(const char*, const char*, uint8_t source)
{
#if HASP_USE_MQTT > 0

    StaticJsonDocument<1024> doc;
    char data[1024];
    char buffer[64];

    doc[F("node")]  = haspDevice.get_hostname();
    doc[F("mdl")]   = haspDevice.get_model();
    doc[F("mf")]    = F(D_MANUFACTURER);
    doc[F("hwid")]  = haspDevice.get_hardware_id();
    doc[F("pages")] = haspPages.count();
    doc[F("sw")]    = haspDevice.get_version();

#if HASP_USE_HTTP > 0
    network_get_ipaddress(buffer, sizeof(buffer));
    doc[F("uri")] = String(F("http://")) + String(buffer);
#elif defined(WINDOWS) || defined(POSIX)
    doc[F("uri")] = "http://google.pt";
#endif

    JsonObject input = doc.createNestedObject(F("input"));
    JsonArray relay  = doc.createNestedArray(F("power"));
    JsonArray led    = doc.createNestedArray(F("light"));
    JsonArray dimmer = doc.createNestedArray(F("dim"));

#if HASP_USE_GPIO > 0
    gpio_discovery(input, relay, led, dimmer);
#endif

    size_t len = serializeJson(doc, data);

    switch(mqtt_send_discovery(data, len)) {
        case MQTT_ERR_OK:
            LOG_TRACE(TAG_MQTT_PUB, F(MQTT_TOPIC_DISCOVERY " => %s"), data);
            break;
        case MQTT_ERR_PUB_FAIL:
            LOG_ERROR(TAG_MQTT_PUB, F(D_MQTT_FAILED " " MQTT_TOPIC_DISCOVERY " => %s"), data);
            break;
        case MQTT_ERR_NO_CONN:
            LOG_ERROR(TAG_MQTT, F(D_MQTT_NOT_CONNECTED));
            break;
        default:
            LOG_ERROR(TAG_MQTT, F(D_ERROR_UNKNOWN));
    }
        // dispatchLastMillis = millis();

#endif
}

// Periodically publish a JSON string indicating system status
void dispatch_statusupdate(const char*, const char*, uint8_t source)
{
#if HASP_USE_MQTT > 0

    char data[400];
    char topic[16];
    {
        char buffer[128];

        hasp_get_sleep_state(topic);
        snprintf_P(data, sizeof(data), PSTR("{\"node\":\"%s\",\"idle\":\"%s\",\"version\":\"%s\",\"uptime\":%lu,"),
                   haspDevice.get_hostname(), topic, haspDevice.get_version(),
                   long(millis() / 1000)); // \"status\":\"available\",

#if HASP_USE_WIFI > 0 || HASP_USE_ETHERNET > 0
        network_get_statusupdate(buffer, sizeof(buffer));
        strcat(data, buffer);

        // snprintf_P(buffer, sizeof(buffer), PSTR("\"mac\":\"%s\","), halGetMacAddress(0, ":").c_str());
        // strcat(data, buffer);
#endif

        snprintf_P(buffer, sizeof(buffer), PSTR("\"heapFree\":%u,\"heapFrag\":%u,\"core\":\"%s\","),
                   haspDevice.get_free_heap(), haspDevice.get_heap_fragmentation(), haspDevice.get_core_version());
        strcat(data, buffer);

        snprintf_P(buffer, sizeof(buffer), PSTR("\"canUpdate\":\"false\",\"page\":%u,\"numPages\":%u,"),
                   haspPages.get(), haspPages.count());
        strcat(data, buffer);

        // #if defined(ARDUINO_ARCH_ESP8266)
        //         snprintf_P(buffer, sizeof(buffer), PSTR("\"espVcc\":%.2f,"), (float)ESP.getVcc() / 1000);
        //         strcat(data, buffer);
        // #endif

        snprintf_P(buffer, sizeof(buffer), PSTR("\"tftDriver\":\"%s\",\"tftWidth\":%u,\"tftHeight\":%u}"),
                   haspTft.get_tft_model(), haspTft.width(), haspTft.height());
        strcat(data, buffer);
    }

    memcpy_P(topic, PSTR("statusupdate"), 13);
    dispatch_state_subtopic(topic, data);
    dispatchSecondsToNextTeleperiod = dispatch_setings.teleperiod;

    /* if(updateEspAvailable) {
            mqttStatusPayload += F("\"updateEspAvailable\":true,");
        } else {
            mqttStatusPayload += F("\"updateEspAvailable\":false,");
        }
    */

#endif
}

void dispatch_current_state(uint8_t source)
{
    dispatch_statusupdate(NULL, NULL, source);
    dispatch_idle(NULL, NULL, source);
    dispatch_current_page();
    dispatch_send_sensordata(NULL, NULL, source);
    dispatch_send_discovery(NULL, NULL, source);
    dispatch_state_antiburn(hasp_get_antiburn());
}

// Format filesystem and erase EEPROM
bool dispatch_factory_reset()
{
    bool formated = true;
    bool erased   = true;

#if HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0
    formated = HASP_FS.format();
    if(formated) filesystemSetupFiles();
#endif

#if HASP_USE_EEPROM > 0
    erased = configClearEeprom();
#endif

    return formated && erased;
}

void dispatch_calibrate(const char*, const char*, uint8_t source)
{
    guiCalibrate();
}

void dispatch_wakeup_obsolete(const char* topic, const char*, uint8_t source)
{
    LOG_WARNING(TAG_MSGR, F(D_ATTRIBUTE_OBSOLETE D_ATTRIBUTE_INSTEAD), topic,
                "idle=off"); // TODO: obsolete dim, light and brightness
    lv_disp_trig_activity(NULL);
    hasp_set_wakeup_touch(false);
}

void dispatch_sleep(const char*, const char*, uint8_t source)
{
    hasp_set_wakeup_touch(false);
}

void dispatch_idle(const char*, const char* payload, uint8_t source)
{
    char topic[6];
    char buffer[6];

    // idle off command
    if(payload && strlen(payload) && !Parser::is_true(payload)) {
        hasp_set_wakeup_touch(false);
        hasp_set_sleep_state(HASP_SLEEP_OFF);
        lv_disp_trig_activity(NULL);
    }

    // idle state
    memcpy_P(topic, PSTR("idle"), 5);
    hasp_get_sleep_state(buffer);
    dispatch_state_subtopic(topic, buffer);
}

void dispatch_reboot(const char*, const char*, uint8_t source)
{
    dispatch_reboot(true);
}

void dispatch_factory_reset(const char*, const char*, uint8_t source)
{
    dispatch_factory_reset();
    delay(500);
    dispatch_reboot(false); // don't save running config
}

void dispatch_theme(const char*, const char* themeid, uint8_t source)
{
    hasp_set_theme(atoi(themeid));
}

void dispatch_service(const char*, const char* payload, uint8_t source)
{
#if HASP_USE_TELNET > 0
    if(!strcmp_P(payload, "start telnet")) {
        telnetStart();
    } else if(!strcmp_P(payload, "stop telnet")) {
        telnetStop();
    }
#endif

#if HASP_USE_FTP > 0
    if(!strcmp_P(payload, "start ftp")) {
        ftpStart();
    } else if(!strcmp_P(payload, "stop ftp")) {
        ftpStop();
    }
#endif

#if HASP_USE_HTTP > 0 || HASP_USE_HTTP_ASYNC > 0
    if(!strcmp_P(payload, "start http")) {
        httpStart();
    } else if(!strcmp_P(payload, "stop http")) {
        httpStop();
    }
#endif

#if ARDUINO && HASP_USE_CONSOLE
    if(!strcmp_P(payload, "start console")) {
        consoleStart();
    } else if(!strcmp_P(payload, "stop console")) {
        consoleStop();
    }
#endif
}

/******************************************* Commands builder *******************************************/

static void dispatch_add_command(const char* p_cmdstr, void (*func)(const char*, const char*, uint8_t))
{
    if(nCommands >= sizeof(commands) / sizeof(haspCommand_t)) {
        LOG_FATAL(TAG_MSGR, F("CMD_OVERFLOW %d"), nCommands); // Needs to be in curly braces
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

    LOG_TRACE(TAG_MSGR, F(D_SERVICE_STARTING));

    /* WARNING: remember to expand the commands array when adding new commands */
    dispatch_add_command(PSTR("json"), dispatch_parse_json);
    dispatch_add_command(PSTR("page"), dispatch_page);
    dispatch_add_command(PSTR("sleep"), dispatch_sleep);
    dispatch_add_command(PSTR("statusupdate"), dispatch_statusupdate);
    dispatch_add_command(PSTR("clearpage"), dispatch_clear_page);
    dispatch_add_command(PSTR("jsonl"), dispatch_parse_jsonl);
    dispatch_add_command(PSTR("backlight"), dispatch_backlight);
    dispatch_add_command(PSTR("moodlight"), dispatch_moodlight);
    dispatch_add_command(PSTR("idle"), dispatch_idle);
    dispatch_add_command(PSTR("dim"), dispatch_backlight_obsolete);        // dim
    dispatch_add_command(PSTR("brightness"), dispatch_backlight_obsolete); // dim
    dispatch_add_command(PSTR("light"), dispatch_backlight_obsolete);
    dispatch_add_command(PSTR("theme"), dispatch_theme);
    dispatch_add_command(PSTR("run"), dispatch_exec);
    dispatch_add_command(PSTR("service"), dispatch_service);
    dispatch_add_command(PSTR("wakeup"), dispatch_wakeup_obsolete);
    dispatch_add_command(PSTR("antiburn"), dispatch_antiburn);
    dispatch_add_command(PSTR("calibrate"), dispatch_calibrate);
    dispatch_add_command(PSTR("update"), dispatch_web_update);
    dispatch_add_command(PSTR("reboot"), dispatch_reboot);
    dispatch_add_command(PSTR("restart"), dispatch_reboot);
    dispatch_add_command(PSTR("screenshot"), dispatch_screenshot);
    dispatch_add_command(PSTR("discovery"), dispatch_send_discovery);
    dispatch_add_command(PSTR("factoryreset"), dispatch_factory_reset);
#if HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0
#if defined(ARDUINO_ARCH_ESP32)
    dispatch_add_command(PSTR("unzip"), filesystemUnzip);
#endif
#endif
#if HASP_USE_CONFIG > 0
    dispatch_add_command(PSTR("setupap"), oobeFakeSetup);
#endif
    /* WARNING: remember to expand the commands array when adding new commands */

    LOG_INFO(TAG_MSGR, F(D_SERVICE_STARTED));
}

IRAM_ATTR void dispatchLoop()
{}

#if 1 || ARDUINO
void dispatchEverySecond()
{
    if(dispatchSecondsToNextTeleperiod > 1) {
        dispatchSecondsToNextTeleperiod--;

    } else if(dispatch_setings.teleperiod > 0 && mqttIsConnected()) {
        dispatch_statusupdate(NULL, NULL, TAG_MSGR);
        dispatch_send_discovery(NULL, NULL, TAG_MSGR);
        dispatch_send_sensordata(NULL, NULL, TAG_MSGR);
        dispatchSecondsToNextTeleperiod = dispatch_setings.teleperiod;
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
            dispatch_statusupdate(NULL, NULL);
        }
    }
}
#endif

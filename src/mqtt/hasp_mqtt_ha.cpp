/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "ArduinoJson.h"
#include "hasp_conf.h"

#if HASP_USE_MQTT > 0

#include "hasp/hasp.h"
#include "hasp/hasp_dispatch.h"
#include "hasp/hasp_parser.h"
#include "dev/device.h"

#include "hasp_mqtt.h"
#include "hasp_mqtt_ha.h"

#define RETAINED true

#if HASP_TARGET_PC
extern std::string mqttNodeTopic;
extern std::string mqttGroupTopic;
#else
extern char mqttNodeTopic[];
extern char mqttGroupTopic[];
#endif

extern bool mqttEnabled;
extern bool mqttHAautodiscover;

char discovery_prefix[] = "homeassistant";

const char FP_MQTT_HA_DEVICE[] PROGMEM       = "device";
const char FP_MQTT_HA_IDENTIFIERS[] PROGMEM  = "ids";
const char FP_MQTT_HA_NAME[] PROGMEM         = "name";
const char FP_MQTT_HA_MODEL[] PROGMEM        = "mdl";
const char FP_MQTT_HA_MANUFACTURER[] PROGMEM = "mf";

#if HASP_TARGET_ARDUINO

#include "hal/hasp_hal.h"

#define HASP_MAC_ADDRESS halGetMacAddress(0, "").c_str()
#define HASP_MAC_ADDRESS_STR halGetMacAddress(0, "")

// #include "PubSubClient.h"
// extern PubSubClient mqttClient;

#else

#define HASP_MAC_ADDRESS "aabbccddeeff"
#define HASP_MAC_ADDRESS_STR "aabbccddeeff"

#endif

void mqtt_ha_send_json(char* topic, JsonDocument& doc)
{
    LOG_VERBOSE(TAG_MQTT_PUB, topic);

    // size_t n;
    // LOG_VERBOSE(TAG_MQTT_PUB, " >>> measureJson & serializeJson start ");
    // long start = millis();
    // mqttClient.beginPublish(topic, measureJson(doc), RETAINED);
    // n = serializeJson(doc, mqttClient);
    // mqttClient.endPublish();
    // LOG_VERBOSE(TAG_MQTT_PUB, " >>> measureJson & serializeJson done, %d bytes in %d millis\n", n, millis() - start);

    // LOG_VERBOSE(TAG_MQTT_PUB, " >>> serializeJson start ");
    // start = millis();
    char buffer[800];
    size_t len = serializeJson(doc, buffer, sizeof(buffer));
    mqttPublish(topic, buffer, len, RETAINED);
    // LOG_VERBOSE(TAG_MQTT_PUB, " >>>  serializeJson done, %d bytes in %d millis\n", n, millis() - start);
}

// adds the device identifiers to the HA MQTT auto-discovery message
void mqtt_ha_add_device_ids(JsonDocument& doc)
{
    JsonObject device = doc.createNestedObject(FPSTR(FP_MQTT_HA_DEVICE));
    JsonArray ids     = device.createNestedArray(FPSTR(FP_MQTT_HA_IDENTIFIERS));
    ids.add(haspDevice.get_hostname());
    ids.add(haspDevice.get_hardware_id());

    device[F("sw")]                        = haspDevice.get_version();
    device[FPSTR(FP_MQTT_HA_NAME)]         = haspDevice.get_hostname();
    device[FPSTR(FP_MQTT_HA_MODEL)]        = F(PIOENV);
    device[FPSTR(FP_MQTT_HA_MANUFACTURER)] = F(D_MANUFACTURER);

    doc[F("~")] = mqttNodeTopic;
}

// adds the name and unique_id to the HA MQTT auto-discovery message
void mqtt_ha_add_unique_id(JsonDocument& doc, char* item)
{
    char buffer[64];

    snprintf_P(buffer, sizeof(buffer), PSTR("%s %s"), haspDevice.get_hostname(), item);
    doc[FPSTR(FP_MQTT_HA_NAME)] = buffer;

    snprintf_P(buffer, sizeof(buffer), PSTR("hasp_%s-%s"), haspDevice.get_hostname(), item);
    doc[F("uniq_id")] = buffer;
}

void mqtt_ha_register_button(uint8_t page, uint8_t id)
{
    StaticJsonDocument<800> doc;
    mqtt_ha_add_device_ids(doc);

    char buffer[128];
    snprintf_P(buffer, sizeof(buffer), PSTR(HASP_OBJECT_NOTATION), page, id);
    doc[F("stype")] = buffer; // subtype = "p0b0"
    snprintf_P(buffer, sizeof(buffer), PSTR("~" MQTT_TOPIC_STATE "/" HASP_OBJECT_NOTATION), page, id);
    doc[F("t")] = buffer; // topic

    doc[F("atype")] = "trigger"; // automation_type

    Parser::get_event_name(HASP_EVENT_DOWN, buffer, sizeof(buffer));
    doc[F("pl")]   = buffer;
    doc[F("type")] = "button_short_press";
    snprintf_P(buffer, sizeof(buffer), PSTR("%s/device_automation/%s/" HASP_OBJECT_NOTATION "_%s/config"),
               discovery_prefix, haspDevice.get_hostname(), page, id, "short_press");
    mqtt_ha_send_json(buffer, doc);

    Parser::get_event_name(HASP_EVENT_UP, buffer, sizeof(buffer));
    doc[F("pl")]   = buffer;
    doc[F("type")] = "button_short_release";
    snprintf_P(buffer, sizeof(buffer), PSTR("%s/device_automation/%s/" HASP_OBJECT_NOTATION "_%s/config"),
               discovery_prefix, haspDevice.get_hostname(), page, id, "short_release");
    mqtt_ha_send_json(buffer, doc);

    Parser::get_event_name(HASP_EVENT_LONG, buffer, sizeof(buffer));
    doc[F("pl")]   = buffer;
    doc[F("type")] = "button_long_press";
    snprintf_P(buffer, sizeof(buffer), PSTR("%s/device_automation/%s/" HASP_OBJECT_NOTATION "_%s/config"),
               discovery_prefix, haspDevice.get_hostname(), page, id, "long_press");
    mqtt_ha_send_json(buffer, doc);

    Parser::get_event_name(HASP_EVENT_RELEASE, buffer, sizeof(buffer));
    doc[F("pl")]   = buffer;
    doc[F("type")] = "button_long_release";
    snprintf_P(buffer, sizeof(buffer), PSTR("%s/device_automation/%s/" HASP_OBJECT_NOTATION "_%s/config"),
               discovery_prefix, haspDevice.get_hostname(), page, id, "long_release");
    mqtt_ha_send_json(buffer, doc);
}

void mqtt_ha_register_switch(uint8_t page, uint8_t id)
{
    StaticJsonDocument<800> doc;
    mqtt_ha_add_device_ids(doc);

    char buffer[128];
    snprintf_P(buffer, sizeof(buffer), PSTR(HASP_OBJECT_NOTATION), page, id);
    doc[F("stype")] = buffer; // subtype = "p0b0"
    snprintf_P(buffer, sizeof(buffer), PSTR("~" MQTT_TOPIC_STATE "/" HASP_OBJECT_NOTATION), page, id);
    doc[F("t")] = buffer; // topic

    doc[F("atype")] = F("binary_sensor"); // automation_type
    doc[F("pl")]    = F("short");         // payload
    doc[F("type")]  = F("button_short_release");

    snprintf_P(buffer, sizeof(buffer), PSTR("%s/device_automation/%s/" HASP_OBJECT_NOTATION "_%s/config"),
               discovery_prefix, haspDevice.get_hostname(), page, id, "short");

    mqtt_ha_send_json(buffer, doc);
}

void mqtt_ha_register_connectivity()
{
    StaticJsonDocument<1024> doc;
    char item[16];
    snprintf_P(item, sizeof(item), PSTR("connectivity"));

    // start from static keys and values that do not change
    deserializeJson(doc, F("{\"device_class\":\"connectivity\",\"stat_t\":\"~LWT\",\"pl_on\":\"online\",\"pl_off\":"
                           "\"offline\",\"json_attr_t\":\"~" MQTT_TOPIC_STATE "/statusupdate\"}"));
    mqtt_ha_add_device_ids(doc);
    mqtt_ha_add_unique_id(doc, item);

    char buffer[128];
    snprintf_P(buffer, sizeof(buffer), PSTR("%s/binary_sensor/%s/%s/config"), discovery_prefix,
               haspDevice.get_hostname(), item);
    mqtt_ha_send_json(buffer, doc);
}

void mqtt_ha_register_backlight()
{
    DynamicJsonDocument doc(1024);
    char item[16];
    snprintf_P(item, sizeof(item), PSTR("backlight"));

    // start from static keys and values that do not change
    deserializeJson(doc, F("{"
                           "\"cmd_t\":\"~" MQTT_TOPIC_COMMAND "/light\","
                           "\"stat_t\":\"~" MQTT_TOPIC_STATE "/light\","
                           "\"pl_on\":\"on\","
                           "\"pl_off\":\"off\","
                           "\"avty_t\":\"~LWT\","
                           "\"bri_stat_t\":\"~" MQTT_TOPIC_STATE "/dim\","
                           "\"bri_cmd_t\":\"~" MQTT_TOPIC_COMMAND "/dim\","
                           "\"bri_scl\":100}"));
    mqtt_ha_add_device_ids(doc);
    mqtt_ha_add_unique_id(doc, item);

    // doc[F("pl_on")]  = F("on");
    // doc[F("pl_off")] = F("off");

    char buffer[128];
    snprintf_P(buffer, sizeof(buffer), PSTR("%s/light/%s/%s/config"), discovery_prefix, haspDevice.get_hostname(),
               item);
    mqtt_ha_send_json(buffer, doc);
}

void mqtt_ha_register_moodlight()
{
    DynamicJsonDocument doc(1024);
    char item[16];
    snprintf_P(item, sizeof(item), PSTR("moodlight"));

    // start from static keys and values that do not change
    deserializeJson(doc, F("{"
                           "\"cmd_t\":\"~" MQTT_TOPIC_COMMAND "/moodlight\","
                           "\"stat_t\":\"~" MQTT_TOPIC_STATE "/moodlight\","
                           "\"platform\":\"mqtt\","
                           "\"schema\":\"json\","
                           "\"rgb\":true,"
                           "\"brightness\":true,"
                           "\"avty_t\":\"~LWT\"}"));

    /*    deserializeJson(doc, F("{"
                               "\"cmd_t\":\"~command/moodlight\","
                               //                      "\"stat_t\":\"~" MQTT_TOPIC_STATE "/moodlight\","
                               "\"avty_t\":\"~LWT\","
                               "\"bri_stat_t\":\"~" MQTT_TOPIC_STATE "/moodlight/dim\","
                               "\"bri_cmd_t\":\"~command/moodlight/dim\","
                               "\"bri_scl\":100,"
                               "\"rgb_stat_t\":\"~" MQTT_TOPIC_STATE "/moodlight/rgb\","
                               "\"rgb_cmd_t\":\"~command/moodlight/rgb\"}"));
                               */
    mqtt_ha_add_device_ids(doc);
    mqtt_ha_add_unique_id(doc, item);

    // doc[F("pl_on")]  = F("ON");
    // doc[F("pl_off")] = F("OFF");

    // doc[F("state_value_template")]      = F("~command/moodlight/light");
    // doc[F("brightness_value_template")] = F("{{ value_json.brightness }}");
    // doc[F("rgb_command_template")]        = F("{{ '%02x%02x%02x0000'| format(red, green, blue) }}");

    char buffer[128];
    snprintf_P(buffer, sizeof(buffer), PSTR("%s/light/%s/%s/config"), discovery_prefix, haspDevice.get_hostname(),
               item);
    mqtt_ha_send_json(buffer, doc);
}

void mqtt_ha_register_idle()
{
    StaticJsonDocument<800> doc;
    char item[16];
    snprintf_P(item, sizeof(item), PSTR("idle"));

    // start from static keys and values that do not change
    deserializeJson(doc, F("{\"stat_t\":\"~" MQTT_TOPIC_STATE
                           "/idle\",\"avty_t\":\"~LWT\",\"json_attr_t\":\"~" MQTT_TOPIC_STATE "/statusupdate\"}"));
    mqtt_ha_add_device_ids(doc);
    mqtt_ha_add_unique_id(doc, item);

    char buffer[128];
    snprintf_P(buffer, sizeof(buffer), PSTR("%s/sensor/%s/%s/config"), discovery_prefix, haspDevice.get_hostname(),
               item);
    mqtt_ha_send_json(buffer, doc);
}

void mqtt_ha_register_activepage()
{
    StaticJsonDocument<800> doc;
    char item[16];
    snprintf_P(item, sizeof(item), PSTR("page"));

    // start from static keys and values that do not change
    deserializeJson(doc, F("{\"cmd_t\":\"~" MQTT_TOPIC_COMMAND "/page\",\"stat_t\":\"~" MQTT_TOPIC_STATE
                           "/page\",\"avty_t\":\"~LWT\"}"));
    mqtt_ha_add_device_ids(doc);
    mqtt_ha_add_unique_id(doc, item);

    char buffer[128];
    snprintf_P(buffer, sizeof(buffer), PSTR("%s/number/%s/%s/config"), discovery_prefix, haspDevice.get_hostname(),
               item);
    mqtt_ha_send_json(buffer, doc);
}

void mqtt_ha_register_auto_discovery()
{
    LOG_TRACE(TAG_MQTT_PUB, F(D_MQTT_HA_AUTO_DISCOVERY));
    mqtt_ha_register_activepage();
    // mqtt_ha_register_button(0, 1);
    // mqtt_ha_register_button(0, 2);
    mqtt_ha_register_backlight();
    mqtt_ha_register_moodlight();
    mqtt_ha_register_idle();
    mqtt_ha_register_connectivity();
}
#endif

/*

homeassistant/binary_sensor/hasptest/idlestate/config

name: hasptest Touch
state_topic: hasp/hasptest/state/idle
value_template: '{% if value == "OFF" %}ON{% else %}OFF{% endif %}'
device_class: moving
json_attributes_topic: hasp/hasptest/state/statusupdate
availability_topic: "hasp/hasptest/LWT"
unique_id: 'hasptest_idlestate'
device:
  identifiers:
    - 'hasptest'
  name: 'HASP Test'
  model: 'hasptest'
  sw_version: 'v0.3.2'
  manufacturer: hasp-lvg


name: 'HASP hasptest Connectivity'
state_topic: "hasp/hasptest/LWT"
payload_on: "online"
payload_off: "offline"
device_class: connectivity
json_attributes_topic: "hasp/hasptest/state/statusupdate"
unique_id: 'hasptest_connectivity'
device:
  identifiers:
    - 'hasptest'
  name: 'HASP Test'
  model: 'hasptest'
  sw_version: 'v0.3.2'
  manufacturer: openHASP


{
    "device":
        {"ids": "plate_87546c", "name": "Test Switchplate", "mdl": "Lanbon L8", "sw": "v0.3.1", "mf": "openHASP"},
    "name": "Backlight",
    "uniq_id": "hasp35_light",
    "~": "hasp/plate35",
    "cmd_t": "~/command/light",
    "stat_t": "~/state/light",
    "avty_t": "~/LWT",
    "bri_stat_t": "~/state/dim",
    "bri_cmd_t": "~/command/dim",
    "bri_scl": 100,
    "pl_on": "on",
    "pl_off": "off"
}

{
    "name": "Lanbon idle State",
    "state_topic": "hasp/lanbon/state/idle",
    "value_template": "{{ value | capitalize }}",
    "icon": "hass:card",
    "json_attributes_topic": "hasp/lanbon/state/statusupdate",
    "availability_topic": "hasp/lanbon/LWT",
    "unique_id": "plate_87546c_idle",
    "device": {
        "identifiers": ["plate_87546c"],
        "name": "Test Switchplate",
        "model": "Lanbon L8",
        "sw_version": "v0.3.1",
        "manufacturer": "openHASP"
    }
}

{
    "name" : "Lanbon Backlight",
             "state_topic" : "hasp/lanbon/state/light",
                             "command_topic" : "hasp/lanbon/command/light",
                                               "brightness_state_topic" : "hasp/lanbon/state/dim",
                                                                          "brightness_scale" : 100,
                                                                          "brightness_command_topic"
        : "hasp/lanbon/command/dim",
          "availability_topic" : "hasp/lanbon/LWT",
                                 "unique_id" : "plate_87546c_bcklght",
                                               "device":
    {
        "identifiers" : ["plate_87546c"]
    }
}
*/

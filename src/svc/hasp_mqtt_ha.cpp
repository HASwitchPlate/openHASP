/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "ArduinoJson.h"
#include "hasp_conf.h"
#if HASP_USE_MQTT > 0

    #include "PubSubClient.h"

    #include "hasp/hasp.h"
    #include "hasp_hal.h"
    #include "hasp_mqtt.h"
    #include "hasp_mqtt_ha.h"

    #define RETAINED true

extern PubSubClient mqttClient;
extern char mqttNodeName[16];
extern char mqttNodeTopic[24];
extern char mqttGroupTopic[24];
extern bool mqttEnabled;
extern bool mqttHAautodiscover;

char discovery_prefix[] = "homeassistant";

void mqtt_ha_add_device(DynamicJsonDocument & doc)
{
    JsonObject device = doc.createNestedObject(F("device"));
    JsonArray ids     = device.createNestedArray(F("ids"));
    ids.add(mqttNodeName);
    ids.add(halGetMacAddress(0, ""));

    char version[32];
    haspGetVersion(version, sizeof(version));
    device[F("sw")] = version;

    device[F("name")] = mqttNodeName;
    device[F("mdl")]  = F(PIOENV);
    device[F("mf")]   = F("hasp-lvgl");

    doc["~"] = mqttNodeTopic;
}

void mqtt_ha_register_button(uint8_t page, uint8_t id)
{
    char buffer[128];
    DynamicJsonDocument doc(512);
    mqtt_ha_add_device(doc);

    snprintf_P(buffer, sizeof(buffer), PSTR("p%db%d"), page, id);
    doc["stype"] = buffer; // subtype = "p0b0"
    snprintf_P(buffer, sizeof(buffer), PSTR("~state/p%db%d"), page, id);
    doc["t"] = buffer; // topic

    doc["atype"] = "trigger"; // automation_type
    doc["pl"]    = "SHORT";   // payload
    doc["type"]  = "button_short_release";

    snprintf_P(buffer, sizeof(buffer), PSTR("%s/device_automation/%s/p%db%d_%s/config"), discovery_prefix, mqttNodeName,
               page, id, "short");

    mqttClient.beginPublish(buffer, measureJson(doc), RETAINED);
    serializeJson(doc, mqttClient);
    mqttClient.endPublish();
}

void mqtt_ha_register_switch(uint8_t page, uint8_t id)
{
    char buffer[128];
    DynamicJsonDocument doc(512);
    mqtt_ha_add_device(doc);

    snprintf_P(buffer, sizeof(buffer), PSTR("p%db%d"), page, id);
    doc["stype"] = buffer; // subtype = "p0b0"
    snprintf_P(buffer, sizeof(buffer), PSTR("~state/p%db%d"), page, id);
    doc["t"] = buffer; // topic

    doc["atype"] = "binary_sensor"; // automation_type
    doc["pl"]    = "SHORT";         // payload
    doc["type"]  = "button_short_release";

    snprintf_P(buffer, sizeof(buffer), PSTR("%s/device_automation/%s/p%db%d_%s/config"), discovery_prefix, mqttNodeName,
               page, id, "short");

    mqttClient.beginPublish(buffer, measureJson(doc), RETAINED);
    serializeJson(doc, mqttClient);
    mqttClient.endPublish();
}

void mqtt_ha_register_connectivity()
{
    char buffer[128];
    DynamicJsonDocument doc(512);
    mqtt_ha_add_device(doc);

    doc[F("device_class")] = F("connectivity");

    snprintf_P(buffer, sizeof(buffer), PSTR("HASP %s %s"), mqttNodeName, doc[F("device_class")].as<String>().c_str());
    doc[F("name")]         = buffer;

    doc[F("stat_t")] = F("~LWT");
    doc[F("pl_on")]  = F("online");
    doc[F("pl_off")] = F("offline");

    doc[F("json_attr_t")] = F("~state/statusupdate");

    snprintf_P(buffer, sizeof(buffer), PSTR("%s_%s"), mqttNodeName, doc[F("device_class")].as<String>().c_str());
    doc[F("uniq_id")] = buffer;

    snprintf_P(buffer, sizeof(buffer), PSTR("%s/binary_sensor/%s/%s/config"), discovery_prefix, mqttNodeName,
               doc[F("device_class")].as<String>().c_str());

    mqttClient.beginPublish(buffer, measureJson(doc), RETAINED);
    serializeJson(doc, mqttClient);
    mqttClient.endPublish();
}

void mqtt_ha_send_backlight()
{
    char component[20];
    char object_id[20]; // object
    char device_id[20];
    char unique_id[20];
    char configtopic[64];
    char payload[512];

    snprintf_P(device_id, sizeof(device_id), PSTR("%s_0123456"), mqttNodeName);

    snprintf_P(component, sizeof(component), PSTR("light"));
    snprintf_P(object_id, sizeof(object_id), PSTR("light"));
    snprintf_P(unique_id, sizeof(unique_id), PSTR("%s_%s"), mqttNodeName, object_id);

    snprintf_P(configtopic, sizeof(configtopic), PSTR("%s/%s/%s/%s/config"), discovery_prefix, component, mqttNodeName,
               object_id);
    snprintf_P(payload, sizeof(payload),
               PSTR("{"
                    "\"device\":{\"ids\":\"%s\",\"name\":\"%s\",\"mdl\":\"Lanbon "
                    "L8\",\"sw\":\"%d.%d.%d\",\"mf\":\"hasp-lvgl\"},"
                    "\"name\":\"Backlight\","
                    "\"uniq_id\":\"%s\","
                    "\"~\":\"%s\","
                    "\"cmd_t\":\"~command/light\","
                    "\"stat_t\":\"~state/light\","
                    "\"avty_t\":\"~LWT\","
                    "\"bri_stat_t\":\"~state/dim\","
                    "\"bri_cmd_t\":\"~command/dim\","
                    "\"bri_scl\":100,"
                    "\"pl_on\":\"ON\","
                    "\"pl_off\":\"OFF\""
                    "}"),
               device_id, mqttNodeName, HASP_VER_MAJ, HASP_VER_MIN, HASP_VER_REV, unique_id, mqttNodeTopic);

    mqttClient.publish(configtopic, payload, RETAINED);

    snprintf_P(component, sizeof(component), PSTR("sensor"));
    snprintf_P(object_id, sizeof(object_id), PSTR("sensor"));
    snprintf_P(unique_id, sizeof(unique_id), PSTR("%s_%s"), mqttNodeName, object_id);

    snprintf_P(configtopic, sizeof(configtopic), PSTR("%s/%s/%s/%s/config"), discovery_prefix, component, mqttNodeName,
               object_id);
    snprintf_P(payload, sizeof(payload),
               PSTR("{"
                    "\"device\":{\"ids\":\"%s\",\"name\":\"%s\",\"mdl\":\"Lanbon "
                    "L8\",\"sw\":\"%d.%d.%d\",\"mf\":\"hasp-lvgl\"},"
                    "\"name\":\"Idle State\","
                    "\"uniq_id\":\"%s\","
                    "\"~\":\"%s\","
                    "\"avty_t\":\"~LWT\","
                    "\"stat_t\":\"~state/idle\","
                    "\"json_attr_t\":\"~state/statusupdate\","
                    "\"val_tpl\":\"{{ value | capitalize }}\""
                    "}"),
               device_id, mqttNodeName, HASP_VER_MAJ, HASP_VER_MIN, HASP_VER_REV, unique_id, mqttNodeTopic);

    mqttClient.publish(configtopic, payload, RETAINED);

    mqtt_ha_register_button(0, 1);
    mqtt_ha_register_button(0, 2);
    mqtt_ha_register_connectivity();
}
#endif

/*
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
  manufacturer: hasp-lvgl


{
    "device":
        {"ids": "plate_87546c", "name": "Test Switchplate", "mdl": "Lanbon L8", "sw": "v0.3.1", "mf": "hasp-lvgl"},
    "name": "Backlight",
    "uniq_id": "hasp35_light",
    "~": "hasp/plate35",
    "cmd_t": "~/command/light",
    "stat_t": "~/state/light",
    "avty_t": "~/LWT",
    "bri_stat_t": "~/state/dim",
    "bri_cmd_t": "~/command/dim",
    "bri_scl": 100,
    "pl_on": "ON",
    "pl_off": "OFF"
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
        "manufacturer": "hasp-lvgl"
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
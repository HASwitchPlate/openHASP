/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "ArduinoJson.h"
#include "hasp_conf.h"
#if HASP_USE_MQTT > 0

    #include "PubSubClient.h"

    #include "hasp/hasp.h"
    #include "hasp/hasp_dispatch.h"
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

void mqtt_ha_send_json(char * topic, DynamicJsonDocument & doc)
{
    mqttClient.beginPublish(topic, measureJson(doc), RETAINED);
    serializeJson(doc, mqttClient);
    mqttClient.endPublish();
}

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

    doc[F("~")] = mqttNodeTopic;
}

void mqtt_ha_register_button(uint8_t page, uint8_t id)
{
    char buffer[128];
    DynamicJsonDocument doc(512);
    mqtt_ha_add_device(doc);

    snprintf_P(buffer, sizeof(buffer), PSTR("p%db%d"), page, id);
    doc[F("stype")] = buffer; // subtype = "p0b0"
    snprintf_P(buffer, sizeof(buffer), PSTR("~state/p%db%d"), page, id);
    doc[F("t")] = buffer; // topic

    doc[F("atype")] = "trigger"; // automation_type

    dispatch_get_event_name(HASP_EVENT_DOWN, buffer, sizeof(buffer));
    doc[F("pl")]   = buffer;
    doc[F("type")] = "button_short_press";
    snprintf_P(buffer, sizeof(buffer), PSTR("%s/device_automation/%s/p%db%d_%s/config"), discovery_prefix, mqttNodeName,
               page, id, "short_press");
    mqtt_ha_send_json(buffer, doc);

    dispatch_get_event_name(HASP_EVENT_SHORT, buffer, sizeof(buffer));
    doc[F("pl")]   = buffer;
    doc[F("type")] = "button_short_release";
    snprintf_P(buffer, sizeof(buffer), PSTR("%s/device_automation/%s/p%db%d_%s/config"), discovery_prefix, mqttNodeName,
               page, id, "short_release");
    mqtt_ha_send_json(buffer, doc);

    dispatch_get_event_name(HASP_EVENT_LONG, buffer, sizeof(buffer));
    doc[F("pl")]   = buffer;
    doc[F("type")] = "button_long_press";
    snprintf_P(buffer, sizeof(buffer), PSTR("%s/device_automation/%s/p%db%d_%s/config"), discovery_prefix, mqttNodeName,
               page, id, "long_press");
    mqtt_ha_send_json(buffer, doc);

    dispatch_get_event_name(HASP_EVENT_UP, buffer, sizeof(buffer));
    doc[F("pl")]   = buffer;
    doc[F("type")] = "button_long_release";
    snprintf_P(buffer, sizeof(buffer), PSTR("%s/device_automation/%s/p%db%d_%s/config"), discovery_prefix, mqttNodeName,
               page, id, "long_release");
    mqtt_ha_send_json(buffer, doc);
}

void mqtt_ha_register_switch(uint8_t page, uint8_t id)
{
    char buffer[128];
    DynamicJsonDocument doc(512);
    mqtt_ha_add_device(doc);

    snprintf_P(buffer, sizeof(buffer), PSTR("p%db%d"), page, id);
    doc[F("stype")] = buffer; // subtype = "p0b0"
    snprintf_P(buffer, sizeof(buffer), PSTR("~state/p%db%d"), page, id);
    doc[F("t")] = buffer; // topic

    doc[F("atype")] = "binary_sensor"; // automation_type
    doc[F("pl")]    = "SHORT";         // payload
    doc[F("type")]  = "button_short_release";

    snprintf_P(buffer, sizeof(buffer), PSTR("%s/device_automation/%s/p%db%d_%s/config"), discovery_prefix, mqttNodeName,
               page, id, "short");

    mqtt_ha_send_json(buffer, doc);
}

void mqtt_ha_register_connectivity()
{
    char buffer[128];
    DynamicJsonDocument doc(512);
    mqtt_ha_add_device(doc);

    doc[F("device_class")] = F("connectivity");

    snprintf_P(buffer, sizeof(buffer), PSTR("HASP %s %s"), mqttNodeName, doc[F("device_class")].as<String>().c_str());
    doc[F("name")] = buffer;

    doc[F("stat_t")] = F("~LWT");
    doc[F("pl_on")]  = F("online");
    doc[F("pl_off")] = F("offline");

    doc[F("json_attr_t")] = F("~state/statusupdate");

    snprintf_P(buffer, sizeof(buffer), PSTR("hasp_%s-%s"), halGetMacAddress(0, "").c_str(),
               doc[F("device_class")].as<String>().c_str());
    doc[F("uniq_id")] = buffer;

    snprintf_P(buffer, sizeof(buffer), PSTR("%s/binary_sensor/%s/%s/config"), discovery_prefix, mqttNodeName,
               doc[F("device_class")].as<String>().c_str());

    mqtt_ha_send_json(buffer, doc);
}

void mqtt_ha_register_backlight()
{
    char buffer[128];
    DynamicJsonDocument doc(640);
    mqtt_ha_add_device(doc);

    snprintf_P(buffer, sizeof(buffer), PSTR("HASP %s backlight"), mqttNodeName);
    doc[F("name")] = buffer;

    doc[F("cmd_t")]  = F("~command/light");
    doc[F("stat_t")] = F("~state/light");
    doc[F("pl_on")]  = F("ON");
    doc[F("pl_off")] = F("OFF");

    doc[F("avty_t")]     = F("~LWT");
    doc[F("bri_stat_t")] = F("~state/dim");
    doc[F("bri_cmd_t")]  = F("~command/dim");
    // doc[F("brightness")]       = true;
    doc[F("bri_scl")] = 100;

    snprintf_P(buffer, sizeof(buffer), PSTR("hasp_%s-backlight"), halGetMacAddress(0, "").c_str(), mqttNodeName);
    doc[F("uniq_id")] = buffer;

    snprintf_P(buffer, sizeof(buffer), PSTR("%s/light/%s/backlight/config"), discovery_prefix, mqttNodeName);

    mqtt_ha_send_json(buffer, doc);
}

void mqtt_ha_register_moodlight()
{
    char buffer[128];
    DynamicJsonDocument doc(1024);
    mqtt_ha_add_device(doc);

    snprintf_P(buffer, sizeof(buffer), PSTR("HASP %s moodlight"), mqttNodeName);
    doc[F("name")] = buffer;

    doc[F("cmd_t")]  = F("~command/moodlight");
    doc[F("stat_t")] = F("~state/moodlight");
    doc[F("pl_on")]  = F("ON");
    doc[F("pl_off")] = F("OFF");

    doc[F("avty_t")]     = F("~LWT");
    doc[F("bri_stat_t")] = F("~state/moodlight/dim");
    doc[F("bri_cmd_t")]  = F("~command/moodlight/dim");
    //doc[F("rgb")]        = true;
    doc[F("bri_scl")]    = 100;

    doc[F("rgb_stat_t")]           = F("~state/moodlight/rgb");
    doc[F("rgb_cmd_t")]         = F("~command/moodlight/rgb");
    // doc[F("state_value_template")]      = F("~command/moodlight/light");
    // doc[F("brightness_value_template")] = F("{{ value_json.brightness }}");
    // doc[F("rgb_command_template")]        = F("{{ '%02x%02x%02x0000'| format(red, green, blue) }}");

    snprintf_P(buffer, sizeof(buffer), PSTR("hasp_%s-moodlight"), halGetMacAddress(0, "").c_str());
    doc[F("uniq_id")] = buffer;

    snprintf_P(buffer, sizeof(buffer), PSTR("%s/light/%s/moodlight/config"), discovery_prefix, mqttNodeName);

    mqtt_ha_send_json(buffer, doc);
}

void mqtt_ha_register_idle()
{
    char buffer[128];
    DynamicJsonDocument doc(640);
    mqtt_ha_add_device(doc);

    snprintf_P(buffer, sizeof(buffer), PSTR("HASP %s idle state"), mqttNodeName);
    doc[F("name")] = buffer;

    // doc[F("cmd_t")]  = F("~command/wakeup");
    doc[F("stat_t")]      = F("~state/idle");
    doc[F("avty_t")]      = F("~LWT");
    doc[F("json_attr_t")] = F("~state/statusupdate");

    snprintf_P(buffer, sizeof(buffer), PSTR("hasp_%s-idlestate"), halGetMacAddress(0, "").c_str(), mqttNodeName);
    doc[F("uniq_id")] = buffer;

    // "value_template" : "{{ value | capitalize }}",
    //                    "icon" : "hass:card",

    snprintf_P(buffer, sizeof(buffer), PSTR("%s/sensor/%s/idlestate/config"), discovery_prefix, mqttNodeName);

    mqtt_ha_send_json(buffer, doc);
}

void mqtt_ha_register_activepage()
{
    char buffer[128];
    DynamicJsonDocument doc(640);
    mqtt_ha_add_device(doc);

    snprintf_P(buffer, sizeof(buffer), PSTR("%s HASP active page"), mqttNodeName);
    doc[F("name")] = buffer;

    doc[F("cmd_t")]       = F("~command/page");
    doc[F("stat_t")]      = F("~state/page");
    doc[F("avty_t")]      = F("~LWT");

    snprintf_P(buffer, sizeof(buffer), PSTR("%s_hasp_activepage"), halGetMacAddress(0, "").c_str(), mqttNodeName);
    doc[F("uniq_id")] = buffer;

    snprintf_P(buffer, sizeof(buffer), PSTR("%s/number/%s/activepage/config"), discovery_prefix, mqttNodeName);

    mqtt_ha_send_json(buffer, doc);
}

void mqtt_ha_send_backlight()
{
    mqtt_ha_register_activepage();
    mqtt_ha_register_button(0, 1);
    mqtt_ha_register_button(0, 2);
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
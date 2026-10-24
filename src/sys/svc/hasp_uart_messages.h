// MIT License. Stock openHASP state JSON to token events; platform-independent.
#pragma once
#include "hasp_uart_framing.h"
#include <ArduinoJson.h>
#include <stdio.h>
#include <initializer_list>
namespace hasp_uart {
inline void forwardState(Outbox& outbox, const char* subtopic, const char* payload) {
    if(!strcmp(subtopic, "page")) {
        // Stock page state is a decimal number. Named pages are not transported.
        if(!*payload) return;
        unsigned page = 0;
        for(const char* p = payload; *p; ++p) {
            if(*p < '0' || *p > '9' || page > 25) return;
            page = page * 10 + (*p - '0');
        }
        if(!page || page > 255) return;
        char number[4];
        snprintf(number, sizeof(number), "%u", page);
        outbox.state("page", number);
        return;
    }
    // Use openHASP's existing JSON dependency only on the panel. Discard all
    // metadata and non-event states; MQTT still receives the original payload.
    StaticJsonDocument<64> filter;
    filter["event"] = true;
    StaticJsonDocument<128> event;
    if(deserializeJson(event, payload, DeserializationOption::Filter(filter))) return;
    if(!event["event"].is<const char*>()) return;
    const char* name = event["event"];
    bool known = false;
    for(const char* value : {"down", "up", "release", "lost", "long", "hold", "changed"})
        if(!strcmp(name, value)) { known = true; break; }
    if(!known) return;
    outbox.event(subtopic, name);
}
} // namespace hasp_uart

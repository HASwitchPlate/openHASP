/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_SLAVE_H
#define HASP_SLAVE_H

#include "ArduinoJson.h"

#define HASP_SLAVE_SPEED 57600

void TASMO_EVERY_SECOND(void);
void TASMO_DATA_RECEIVE(char *data);
void slave_send_state(const __FlashStringHelper * subtopic, const char * payload);
void slave_send_obj_attribute_str(uint8_t pageid, uint8_t btnid, const char * attribute, const char * data);
void slave_send_input(uint8_t id, const char * payload);
void slave_send_statusupdate();


void slaveSetup();
void IRAM_ATTR slaveLoop(void);


#endif
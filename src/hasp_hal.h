#ifndef HASP_HAL_H
#define HASP_HAL_H

#include <Arduino.h>

uint8_t halGetHeapFragmentation(void);
String halGetResetInfo(void);
size_t halGetMaxFreeBlock(void);
size_t halGetFreeHeap(void);
String halGetCoreVersion(void);
String halGetChipModel();
void halRestart(void);

#endif
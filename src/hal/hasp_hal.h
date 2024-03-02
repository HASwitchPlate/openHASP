/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_HAL_H
#define HASP_HAL_H

#include <Arduino.h>

// void halRestartMcu(void);
// String halGetResetInfo(void);
// uint8_t halGetHeapFragmentation(void);
// size_t halGetMaxFreeBlock(void);
// size_t halGetFreeHeap(void);
// String halGetCoreVersion(void);
// String halGetChipModel();
String halGetMacAddress(int start, const char* separator);
// uint16_t halGetCpuFreqMHz(void);
// String halDisplayDriverName(void);
// String halGpioName(uint8_t gpio);

#endif
/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_DEVICE_LANBONL8_H
#define HASP_DEVICE_LANBONL8_H

#include "esp32.h"

#if defined(LANBONL8)

void Read_PCNT();

namespace dev {

class LanbonL8 : public Esp32Device {
  public:
    void init();
    void loop_5s();
    void get_sensors(JsonDocument& doc);
};

} // namespace dev

using dev::LanbonL8;
extern dev::LanbonL8 haspDevice;

#endif
#endif